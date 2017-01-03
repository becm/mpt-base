/*!
 * dispatch event or reply from connection input
 */

/* request format definitions */
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>

#include "output.h"

#include "message.h"
#include "stream.h"

static int connectionLog(MPT_STRUCT(connection) *con, const char *from, int type, const char *fmt, ...)
{
	va_list va;
	if (fmt) {
		int ret;
		va_start(va, fmt);
		ret = mpt_connection_log(con, from, type | MPT_ENUM(LogPretty) | MPT_ENUM(LogFunction), fmt, va);
		va_end(va);
		return ret;
	}
	return mpt_connection_log(con, from, type, fmt, va);
}
static int replySet(void *ptr, const MPT_STRUCT(message) *src)
{
	static const char _func[] = "mpt::connection::reply";
	
	MPT_STRUCT(connection) *con;
	MPT_STRUCT(reply_context) *rc = ptr;
	uint64_t id = 0;
	int ret;
	
	/* bad context state */
	if (!rc->used) {
		mpt_log(0, _func, MPT_LOG(Critical), "%s %s",
		        MPT_tr("unable to reply"), id, MPT_tr("reply context not registered"));
		return MPT_REPLY(BadContext);
	}
	
	if (!(con = rc->ptr)) {
		mpt_log(0, _func, MPT_LOG(Error), "%s: %s",
		        MPT_tr("unable to reply"), MPT_tr("output destroyed"));
		if (!--rc->used) {
			free(rc);
		}
		return MPT_REPLY(BadDescriptor);
	}
	/* already answered */
	if (!rc->len) {
		connectionLog(con, _func, MPT_LOG(Warning), "%s: %s",
		              MPT_tr("bad reply operation"), MPT_tr("reply already sent"));
		--rc->used;
		return MPT_REPLY(BadState);
	}
	rc->_val[0] &= 0x7f;
	mpt_message_buf2id(rc->_val, rc->len, &id);
	rc->_val[0] |= 0x80;
	if (!MPT_socket_active(&con->out.sock)) {
		MPT_STRUCT(stream) *srm;
		
		if (!(srm = (void *) con->out.buf._buf)) {
			connectionLog(con, _func, MPT_LOG(Error), "%s (%04"PRIx64"): %s",
			              MPT_tr("unable to reply"), id, MPT_tr("no target descriptor"));
			return MPT_ERROR(BadArgument);
		}
		if (mpt_stream_flags(&srm->_info) & MPT_ENUM(StreamMesgAct)) {
			connectionLog(con, _func, MPT_LOG(Error), "%s (%04"PRIx64"): %s",
			              MPT_tr("unable to reply"), id, MPT_tr("message creation in progress"));
			return MPT_ERROR(BadArgument);
		}
		if ((ret = mpt_stream_push(srm, rc->len, rc->_val)) < 0) {
			connectionLog(con, _func, MPT_LOG(Error), "%s (%04"PRIx64"): %s",
			              MPT_tr("bad reply operation"), id, MPT_tr("unable to start reply"));
			return ret;
		}
		if (src && mpt_stream_append(srm, src) < 0) {
			connectionLog(con, _func, MPT_LOG(Warning), "%s (%04"PRIx64"): %s",
			              MPT_tr("bad reply operation"), id, MPT_tr("unable to append message"));
		}
		if ((ret = mpt_stream_push(srm, 0, 0)) < 0) {
			connectionLog(con, _func, MPT_LOG(Warning), "%s (%04"PRIx64"): %s",
			              MPT_tr("bad reply operation"), id, MPT_tr("unable to terminate reply"));
			if (mpt_stream_push(srm, 1, 0) < 0) {
				return ret;
			}
		}
	}
	else {
		uint8_t buf[0x10000];
		struct sockaddr *sa = 0;
		int slen;
		
		ret = con->out._idlen;
		memcpy(buf, rc->_val, ret);
		if (src) {
			MPT_STRUCT(message) msg = *src;
			ret += mpt_message_read(&msg, sizeof(buf) - ret, buf + ret);
			if (mpt_message_length(&msg)) {
				ret = con->out._idlen;
				connectionLog(con, _func, MPT_LOG(Error), "%s (%04"PRIx64"): %s",
				              MPT_tr("unable to reply"), id, MPT_tr("send failed"));
			}
		}
		slen = rc->len - con->out._idlen;
		if (slen) {
			sa = (void *) (rc->_val + con->out._idlen);
		}
		ret = sendto(con->out.sock._id, buf, ret, 0, sa, slen);
		
		if (ret < 0) {
			connectionLog(con, _func, MPT_LOG(Error), "%s (%04"PRIx64"): %s",
			              MPT_tr("unable to reply"), id, MPT_tr("send failed"));
			return MPT_ERROR(BadArgument);
		}
	}
	rc->len = 0;
	--rc->used;
	
	return ret;
}
struct _streamWrapper
{
	MPT_STRUCT(connection) *con;
	MPT_TYPE(EventHandler) cmd;
	void *arg;
};
int streamWrapper(void *ptr, MPT_STRUCT(event) *ev)
{
	struct _streamWrapper *wd = ptr;
	MPT_STRUCT(reply_context) *rc;
	MPT_STRUCT(connection) *con;
	MPT_STRUCT(message) msg;
	
	con = wd->con;
	
	/* reply message indicated */
	if (!ev->reply.context) {
		MPT_STRUCT(command) *ans;
		if (!(ans = mpt_command_get(&con->_wait, ev->id))) {
			connectionLog(con, __func__, MPT_LOG(Error), "%s: %s",
			              MPT_tr("dispatch failed"), MPT_tr("message id incomplete"));
			return MPT_ERROR(BadValue);
		}
		return ans->cmd(ans->arg, (void *) ev->msg);
	}
	if (!(rc = mpt_reply_reserve(&con->_rctx, con->out._idlen))) {
		connectionLog(con, __func__, MPT_LOG(Error), "%s: %s",
		              MPT_tr("dispatch failed"), MPT_tr("no reply context available"));
		return MPT_ERROR(BadArgument);
	}
	msg = *ev->msg;
	if (mpt_message_read(&msg, rc->len, rc->_val) < rc->len) {
		connectionLog(con, __func__, MPT_LOG(Error), "%s: %s",
		              MPT_tr("dispatch failed"), MPT_tr("message id incomplete"));
		return MPT_ERROR(BadArgument);
	}
	rc->ptr = con;
	rc->len = con->out._idlen;
	
	ev->id = 0;
	ev->reply.set = replySet;
	ev->reply.context = rc;
	
	return wd->cmd(wd->arg, ev);
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
	const struct sockaddr *sa;
	MPT_STRUCT(buffer) *buf;
	MPT_STRUCT(message) msg;
	MPT_STRUCT(event) ev;
	uint8_t ilen, slen, *data;
	int ret;
	
	/* message transfer in progress */
	if (con->out.state & MPT_ENUM(OutputActive)) {
		return MPT_ENUM(EventRetry);
	}
	if (!MPT_socket_active(&con->out.sock)) {
		MPT_STRUCT(stream) *srm;
		
		if (!(buf = con->out.buf._buf)) {
			return MPT_ERROR(BadArgument);
		}
		srm = (void *) buf;
		if ((ilen = con->out._idlen)) {
			struct _streamWrapper sw;
			
			sw.con = con;
			sw.arg = arg;
			sw.cmd = cmd;
			
			return mpt_stream_dispatch(srm, 0, streamWrapper, &sw);
		}
		return mpt_stream_dispatch(srm, ilen, cmd, arg);
	}
	/* no new data present */
	if (!(con->out.state & MPT_ENUM(OutputReceived))) {
		return MPT_ENUM(EventRetry);
	}
	con->out.state &= ~MPT_ENUM(OutputReceived);
	
	/* initialize message and event data */
	msg.base = 0;
	msg.used = 0;
	msg.cont = 0;
	msg.clen = 0;
	
	ev.id = 0;
	ev.msg = &msg;
	ev.reply.set = 0;
	ev.reply.context = 0;
	
	/* no message data */
	if (!(buf = con->out.buf._buf)) {
		return cmd(arg, &ev);
	}
	msg.base = data = (void *) (buf + 1);
	msg.used = buf->used;
	buf->used = 0;
	
	/* trailing socket address */
	if ((slen = con->out._scurr)) {
		if (msg.used < slen) {
			connectionLog(con, __func__, MPT_LOG(Error), "%s: %s (%d)",
			              MPT_tr("bad state"), MPT_tr("invalid socket data size"), slen);
			return MPT_ERROR(MissingBuffer);
		}
		msg.used -= slen;
		sa = (void *) (data + msg.used);
	} else {
		sa = 0;
	}
	/* no answer message possible */
	if (!(ilen = con->out._idlen)) {
		return cmd(arg, &ev);
	}
	if ((ret = mpt_message_read(&msg, ilen, 0)) < ilen) {
		connectionLog(con, __func__, MPT_LOG(Error), "%s: %s (%d < %d)",
		              MPT_tr("bad message"), MPT_tr("missing message id"), ret, ilen);
		return MPT_ERROR(MissingData);
	}
	/* match message reply flag */
	if (data[0] & 0x80) {
		MPT_STRUCT(command) *ans;
		uint64_t ansid;
		
		data[0] &= 0x7f;
		ret = mpt_message_buf2id(data, ilen, &ansid);
		if (ret < 0 || ret > (int) sizeof(ans->id)) {
			connectionLog(con, __func__, MPT_LOG(Error), "%s: %s (%d)",
			              MPT_tr("reply failed"), MPT_tr("bad id size"), ret);
			return MPT_ERROR(MissingBuffer);
		}
		if (!(ans = mpt_command_get(&con->_wait, ansid))) {
			connectionLog(con, __func__, MPT_LOG(Error), "%s: %s ("PRIx64")",
			              MPT_tr("reply failed"), MPT_tr("message not registered"), ansid);
			return MPT_ERROR(MissingBuffer);
		}
		if (ans->cmd(ans->arg, &msg) < 0) {
			connectionLog(con, __func__, MPT_LOG(Error), "%s: %s",
			              MPT_tr("dispatch failed"), MPT_tr("no message available"));
			return MPT_ERROR(MissingBuffer);
		}
		return 0;
	}
	ret = 0;
	
	if (cmd) {
		MPT_STRUCT(reply_context) *rc;
		
		rc = mpt_reply_reserve(&con->_rctx, ilen += con->out._scurr);
		if (rc) {
			rc->ptr = con;
			rc->len = ilen;
			memcpy(rc->_val, data, con->out._idlen);
			if (con->out._scurr) {
				memcpy(rc->_val + con->out._idlen, sa, con->out._scurr);
			}
			ev.reply.set = replySet;
			ev.reply.context = rc;
			
			if ((ret = cmd(arg, &ev)) >= 0) {
				return ret;
			}
			else {
				ret = MPT_ENUM(EventFail);
			}
		}
	}
	data[0] |= 0x80;
	if (sendto(con->out.sock._id, data, con->out._idlen, 0, sa, slen)) {
		connectionLog(con, __func__, MPT_LOG(Error), "%s: %s",
		              MPT_tr("reply failed"), MPT_tr("send operation"));
	}
	return ret;
}
