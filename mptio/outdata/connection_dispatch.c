/*!
 * dispatch event or reply from connection input
 */

/* request format definitions */
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <string.h>

#include <sys/uio.h>
#include <arpa/inet.h>

#include "array.h"
#include "queue.h"
#include "event.h"

#include "message.h"

#include "stream.h"

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
	uint64_t id = 0;
	int ret;
	
	mpt_message_buf2id(rc->_val, rc->len, &id);
	
	if (!(con = rc->ptr)) {
		mpt_log(0, __func__, MPT_FCNLOG(Error), "%s (%04"PRIx64"): %s",
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
	/* mark as reply */
	rc->_val[0] |= 0x80;
	
	/* start with raw push of message ID */
	if ((ret = mpt_outdata_push(&con->out, rc->len, rc->_val)) < 0
	    || (ret = mpt_outdata_send(&con->out, src)) < 0) {
		connectionLog(con, __func__, MPT_FCNLOG(Error), "%s (%04x): %s",
			      MPT_tr("unable to reply"), id, MPT_tr("send failed"));
		return MPT_ERROR(BadArgument);
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
	MPT_STRUCT(buffer) *buf;
	MPT_STRUCT(message) msg;
	MPT_STRUCT(event) ev;
	MPT_STRUCT(reply_context) *rc = 0;
	size_t ilen;
	int ret;
	
	ilen = con->out._idlen;
	
	if (!MPT_socket_active(&con->out.sock)) {
		MPT_STRUCT(stream) *srm;
		
		if (!(srm = con->out._buf)) {
			return MPT_ERROR(BadArgument);
		}
		if (ilen) {
			if (!(rc = mpt_reply_reserve(&con->out._ctx, ilen))) {
				connectionLog(con, __func__, MPT_FCNLOG(Error), "%s: %s",
					      MPT_tr("dispatch failed"), MPT_tr("no reply context available"));
				return MPT_ERROR(BadArgument);
			}
			rc->len = ilen;
			rc->used = 1;
		}
		return mpt_stream_dispatch(srm, rc, cmd, arg);
	}
	/* message transfer in progress */
	if (con->out.state & MPT_ENUM(OutputActive)) {
		return MPT_ENUM(EventRetry);
	}
	if (!(buf = con->out._buf) || buf->used < ilen) {
		return MPT_ERROR(MissingData);
	}
	if (ilen) {
		if (!(rc = mpt_reply_reserve(&con->out._ctx, ilen))) {
			connectionLog(con, __func__, MPT_FCNLOG(Error), "%s: %s",
				      MPT_tr("dispatch failed"), MPT_tr("no reply context available"));
			return MPT_ERROR(BadArgument);
		}
		rc->len = ilen;
		rc->used = 1;
		memcpy(rc->_val, buf+1, ilen);
	}
	if (!cmd) {
		/* TODO: discard input data */
		if (rc) {
			mpt_outdata_push(&con->out, rc->len, rc->_val);
			mpt_outdata_push(&con->out, 0, 0);
		}
		return 0;
	}
	msg.base = ((uint8_t *) (buf+1)) + ilen;
	msg.used = buf->used - ilen;
	msg.cont = 0;
	msg.clen = 0;
	
	
	ev.id = 0;
	ev.msg = &msg;
	ev.reply.set = rc ? replySet : 0;
	ev.reply.context = rc;
	
	ret = cmd(arg, &ev);
	
	return ret < 0 ? MPT_ENUM(EventFail) : 0;
}
