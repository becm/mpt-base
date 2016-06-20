/*!
 * dispatch event or reply from connection input
 */

#include <string.h>

#include <sys/uio.h>
#include <arpa/inet.h>

#include "array.h"
#include "queue.h"
#include "event.h"

#include "message.h"

#include "output.h"

static int connectionLog(MPT_STRUCT(connection) *con, const char *from, int type, const char *fmt, ...)
{
	va_list va;
	if (!fmt) {
		int ret;
		va_start(va, fmt);
		ret = mpt_connection_log(con, from, type, fmt, va);
		va_end(va);
		return ret;
	}
	return mpt_connection_log(con, from, type, fmt, va);
}
static int replySet(void *ptr, const MPT_STRUCT(message) *src)
{
	MPT_STRUCT(connection) *con;
	MPT_STRUCT(reply_context) *rc = ptr;
	uint16_t id, orgid;
	int ret;
	
	id = ntohs(*((uint16_t *) rc->_val));
	
	if (!(con = rc->ptr)) {
		mpt_log(0, __func__, MPT_FCNLOG(Error), "%s (%04x): %s",
		        MPT_tr("unable to reply"), id, MPT_tr("processed reply detected"));
		return -3;
	}
	/* already answered */
	if (!rc->len) {
		connectionLog(con, __func__, MPT_FCNLOG(Critical), "%s (%04x): %s",
			      MPT_tr("unable to reply"), id, MPT_tr("processed reply detected"));
		return -3;
	}
	if (con->out.state & MPT_ENUM(OutputActive)) {
		connectionLog(con, __func__, MPT_FCNLOG(Error), "%s (%04x): %s",
			      MPT_tr("unable to reply"), id, MPT_tr("message in progress"));
		return -1;
	}
	orgid = con->cid;
	con->cid = id | 0x8000;
	ret = mpt_connection_send(con, src);
	con->cid = orgid;
	
	if (ret < 0) {
		connectionLog(con, __func__, MPT_FCNLOG(Error), "%s (%04x): %s",
			      MPT_tr("unable to reply"), id, MPT_tr("send failed"));
	}
	return ret;
}

/*!
 * \ingroup mptOutput
 * \brief dispatch data on connection
 * 
 * Decode and dispatch messages on connection input.
 * 
 * Process reply messages until new message arrives.
 * 
 * \param con  connection descriptor
 * \param src  message to send
 */
extern int mpt_connection_dispatch(MPT_STRUCT(connection) *con, MPT_TYPE(EventHandler) cmd, void *arg)
{
	MPT_STRUCT(message) msg;
	struct iovec vec;
	ssize_t len;
	int ret;
	uint8_t buf[4];
	int16_t id;
	
	/* message trnasfer in progress */
	if (con->out.state & MPT_ENUM(OutputActive)) {
		return MPT_ENUM(EventRetry);
	}
	if (!con->in._dec) {
		return MPT_ERROR(BadEncoding);
	}
	/* get next message */
	if ((len = mpt_queue_recv(&con->in)) < 0) {
		return len;
	}
	mpt_queue_crop(&con->in.data, 0, con->in._state.done);
	con->in._state.done = 0;
	mpt_message_get(&con->in.data, 0, len, &msg, &vec);
	
	/* remove message id */
	if (mpt_message_read(&msg, sizeof(buf), buf) < sizeof(id)) {
		return MPT_ERROR(MissingData);
	}
	id = 0x7fff & *((uint16_t *) buf);
	
	/* process reply */
	if (buf[0] & 0x80) {
		MPT_STRUCT(command) *ans;
		
		if ((ans = mpt_command_get(&con->_wait, id))) {
			if (ans->cmd(ans->arg, &msg) < 0) {
				connectionLog(con, __func__, MPT_FCNLOG(Warning), "%s: %04x",
				              MPT_tr("reply processing error"), id);
			}
			ans->cmd = 0;
		} else {
			connectionLog(con, __func__, MPT_FCNLOG(Error), "%s: %04x",
			              MPT_tr("unregistered reply id"), id);
		}
	}
	/* process regular input */
	else {
		MPT_STRUCT(event) ev;
		
		/* event setup */
		ev.id = 0;
		ev.msg = &msg;
		
		/* force remote message */
		if (id) {
			MPT_STRUCT(reply_context) *rc;
			rc = mpt_reply_reserve(&con->_ctx, sizeof(id));
			
			ev.reply.set = 0;
			ev.reply.context = rc;
			
			if (rc) {
				ev.reply.set = replySet;
				
				rc->ptr = con;
				rc->len = sizeof(id);
				rc->used = 1;
				memcpy(rc->_val, buf, sizeof(id));
			}
		}
		/* dispatch data to command */
		if (!cmd) {
			if (ev.reply.set) {
				ev.reply.set(ev.reply.context, 0);
			}
			ret = 0;
		}
		else if ((ret = cmd(arg, &ev)) < 0) {
			ret = MPT_ENUM(EventCtlError);
		} else {
			ret &= MPT_ENUM(EventFlags);
		}
	}
	/* further message on queue */
	if ((len = mpt_queue_peek(&con->in, sizeof(buf), buf)) >= 0) {
		ret |= MPT_ENUM(EventRetry);
	}
	return ret;
}
