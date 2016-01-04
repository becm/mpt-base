/*!
 * finalize connection data
 */

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

struct replyData
{
	MPT_STRUCT(connection) *con;
	uint16_t id;
};
static int replySet(void *ptr, const MPT_STRUCT(message) *src)
{
	MPT_STRUCT(connection) *con;
	struct replyData *rd = ptr;
	uint16_t orgid;
	int ret;
	
	/* already answered */
	if (!rd->id) {
		connectionLog(rd->con, __func__, MPT_FCNLOG(Warning), "%s (%04x): %s",
			      MPT_tr("unable to reply"), rd->id, MPT_tr("processed reply detected"));
		return -3;
	}
	con = rd->con;
	if (con->out.state & MPT_ENUM(OutputActive)) {
		connectionLog(con, __func__, MPT_FCNLOG(Error), "%s (%04x): %s",
			      MPT_tr("unable to reply"), rd->id, MPT_tr("message in progress"));
		return -1;
	}
	orgid = con->cid;
	con->cid = rd->id | 0x8000;
	ret = mpt_connection_send(con, src);
	con->cid = orgid;
	
	if (ret < 0) {
		connectionLog(con, __func__, MPT_FCNLOG(Error), "%s (%04x): %s",
			      MPT_tr("unable to reply"), rd->id, MPT_tr("send failed"));
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
	struct replyData rd;
	MPT_STRUCT(event) ev;
	MPT_STRUCT(message) msg;
	struct iovec vec;
	ssize_t len;
	size_t off;
	int ret;
	uint16_t rid;
	
	/* message trnasfer in progress */
	if (con->out.state & MPT_ENUM(OutputActive)) {
		return MPT_ENUM(EventRetry);
	}
	if (!con->in.dec) {
		return MPT_ERROR(BadEncoding);
	}
	/* get next message */
	if ((len = mpt_queue_recv(&con->in.data, &con->in.info, con->in.dec)) < 0) {
		return len;
	}
	off = con->in.info.done;
	mpt_message_get(&con->in.data, off, len, &msg, &vec);
	
	/* remove message id */
	if (mpt_message_read(&msg, sizeof(rid), &rid) < sizeof(rid)) {
		if (off) {
			mpt_queue_crop(&con->in.data, 0, off);
			con->in.info.done = 0;
		}
		return MPT_ERROR(MissingData);
	}
	rid = ntohs(rid);
	
	/* process reply */
	if (rid & 0x8000) {
		MPT_STRUCT(command) *ans;
		
		rid &= 0x7fff;
		ev.id = rid;
		ev.msg = 0;
		ev.reply.set = 0;
		ev.reply.context = 0;
		
		if ((ans = mpt_command_get(&con->_wait, rid))) {
			if (ans->cmd(ans->arg, &msg) < 0) {
				connectionLog(con, __func__, MPT_FCNLOG(Warning), "%s: %04x",
				              MPT_tr("reply processing error"), rid);
			}
			ans->cmd = 0;
		} else {
			ev.msg = &msg;
			connectionLog(con, __func__, MPT_FCNLOG(Error), "%s: %04x",
			              MPT_tr("unregistered reply id"), rid);
		}
		rd.id = 0;
	}
	/* process regular input */
	else {
		/* event setup */
		ev.id = 0;
		ev.msg = &msg;
		
		/* force remote message */
		if (rid) {
			ev.reply.set = replySet;
			ev.reply.context = &rd;
			
			rd.con = con;
			rd.id  = rid;
		}
	}
	/* dispatch data to command */
	if (!cmd) {
		ret = 0;
	}
	else if ((ret = cmd(arg, &ev)) < 0) {
		ret = MPT_ENUM(EventCtlError);
	} else {
		ret &= MPT_ENUM(EventFlags);
	}
	if (rd.id) {
		replySet(&rd, 0);
	}
	/* remove message data from queue */
	if (off) {
		mpt_queue_crop(&con->in.data, 0, off);
		con->in.info.done = 0;
	}
	/* further message on queue */
	if ((len = mpt_queue_peek(&con->in.data, &con->in.info, con->in.dec, 0)) >= 0) {
		ret |= MPT_ENUM(EventRetry);
	}
	return ret;
}
