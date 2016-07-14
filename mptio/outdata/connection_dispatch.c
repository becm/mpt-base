/*!
 * dispatch event or reply from connection input
 */

/* request format definitions */
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <stdlib.h>
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
	static const char _func[] = "mpt::connection::reply";
	
	MPT_STRUCT(connection) *con;
	MPT_STRUCT(reply_context) *rc = ptr;
	uint64_t id = 0;
	int ret;
	
	/* bad context state */
	if (!rc->used) {
		mpt_log(0, _func, MPT_FCNLOG(Critical), "%s %s",
		        MPT_tr("unable to reply"), id, MPT_tr("reply context not registered"));
		return 0;
	}
	rc->_val[0] &= 0x7f;
	mpt_message_buf2id(rc->_val, rc->len, &id);
	rc->_val[0] |= 0x80;
	
	if (!(con = rc->ptr)) {
		mpt_log(0, _func, MPT_FCNLOG(Error), "%s (%04"PRIx64"): %s",
		        MPT_tr("unable to reply"), id, MPT_tr("context destroyed"));
		if (!--rc->used) {
			free(rc);
		}
		return MPT_ERROR(BadArgument);
	}
	/* already answered */
	if (!rc->len) {
		connectionLog(con, _func, MPT_FCNLOG(Warning), "%s (%04"PRIx64"): %s",
		              MPT_tr("bad reply operation"), id, MPT_tr("reply already sent"));
		--rc->used;
		return 0;
	}
	if (!MPT_socket_active(&con->out.sock)) {
		MPT_STRUCT(stream) *srm;
		
		if (!(srm = con->out._buf)) {
			connectionLog(con, _func, MPT_FCNLOG(Error), "%s (%04"PRIx64"): %s",
			              MPT_tr("unable to reply"), id, MPT_tr("no target descriptor"));
			return MPT_ERROR(BadArgument);
		}
		if (mpt_stream_flags(&srm->_info) & MPT_ENUM(StreamMesgAct)) {
			connectionLog(con, _func, MPT_FCNLOG(Error), "%s (%04"PRIx64"): %s",
			              MPT_tr("unable to reply"), id, MPT_tr("message creation in progress"));
			return MPT_ERROR(BadArgument);
		}
		if ((ret = mpt_stream_push(srm, rc->len, rc->_val)) < 0) {
			connectionLog(con, _func, MPT_FCNLOG(Error), "%s (%04"PRIx64"): %s",
			              MPT_tr("bad reply operation"), id, MPT_tr("unable to start reply"));
			return ret;
		}
		if (src && mpt_stream_send(srm, src) < 0) {
			connectionLog(con, _func, MPT_FCNLOG(Warning), "%s (%04"PRIx64"): %s",
			              MPT_tr("bad reply operation"), id, MPT_tr("unable to append message"));
		}
		if ((ret = mpt_stream_push(srm, 0, 0)) < 0) {
			connectionLog(con, _func, MPT_FCNLOG(Warning), "%s (%04"PRIx64"): %s",
			              MPT_tr("bad reply operation"), id, MPT_tr("unable to terminate reply"));
			if (mpt_stream_push(srm, 1, 0) < 0) {
				return ret;
			}
		}
	}
	else {
		uint8_t buf[0x10000];
		struct sockaddr *sa = 0;
		memcpy(buf, rc->_val, ret = rc->len);
		if (src) {
			MPT_STRUCT(message) msg = *src;
			ret += mpt_message_read(&msg, sizeof(buf) - rc->len, buf + rc->len);
			if (mpt_message_length(&msg)) {
				ret = rc->len;
				connectionLog(con, _func, MPT_FCNLOG(Error), "%s (%04"PRIx64"): %s",
				              MPT_tr("unable to reply"), id, MPT_tr("send failed"));
			}
		}
		if (con->out._socklen) {
			int pos = rc->len;
			sa = (void *) (rc->_val + MPT_align(pos));
		}
		ret = sendto(con->out.sock._id, buf, ret, 0, sa, con->out._socklen);
		
		if (ret < 0) {
			connectionLog(con, _func, MPT_FCNLOG(Error), "%s (%04"PRIx64"): %s",
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
			connectionLog(con, __func__, MPT_FCNLOG(Error), "%s: %s",
			              MPT_tr("dispatch failed"), MPT_tr("message id incomplete"));
			return MPT_ERROR(BadValue);
		}
		return ans->cmd(ans->arg, (void *) ev->msg);
	}
	if (!(rc = mpt_reply_reserve(&con->out._ctx, con->out._idlen))) {
		connectionLog(con, __func__, MPT_FCNLOG(Error), "%s: %s",
		              MPT_tr("dispatch failed"), MPT_tr("no reply context available"));
		return MPT_ERROR(BadArgument);
	}
	rc->ptr = con;
	rc->len = con->out._idlen;
	
	mpt_message_id2buf(ev->id, rc->_val, rc->len);
	
	if (mpt_message_read(&msg, rc->len, rc->_val) < rc->len) {
		connectionLog(con, __func__, MPT_FCNLOG(Error), "%s: %s",
		              MPT_tr("dispatch failed"), MPT_tr("message id incomplete"));
		return MPT_ERROR(BadArgument);
	}
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
	MPT_STRUCT(reply_context) *rc;
	MPT_STRUCT(buffer) *buf;
	MPT_STRUCT(message) msg;
	MPT_STRUCT(event) ev;
	size_t ilen;
	int ret;
	
	/* message transfer in progress */
	if (con->out.state & MPT_ENUM(OutputActive)) {
		return MPT_ENUM(EventRetry);
	}
	if (!MPT_socket_active(&con->out.sock)) {
		MPT_STRUCT(stream) *srm;
		
		if (!(srm = con->out._buf)) {
			return MPT_ERROR(BadArgument);
		}
		if ((ilen = con->out._idlen)) {
			struct _streamWrapper sw;
			
			sw.con = con;
			sw.arg = arg;
			sw.cmd = cmd;
			
			return mpt_stream_dispatch(srm, ilen, streamWrapper, &sw);
		}
		return mpt_stream_dispatch(srm, ilen, cmd, arg);
	}
	/* no new data present */
	if (!(con->out.state & MPT_ENUM(OutputReceived))) {
		return MPT_ENUM(EventRetry);
	}
	if ((buf = con->out._buf)) {
		msg.base = buf + 1;
		msg.used = buf->used;
	} else {
		msg.base = 0;
		msg.used = 0;
	}
	msg.cont = 0;
	msg.clen = 0;
	
	ev.id = 0;
	ev.msg = &msg;
	ev.reply.set = 0;
	ev.reply.context = 0;
	
	con->out.state &= ~MPT_ENUM(OutputReceived);
	
	if (!(ret = con->_ctxpos)) {
		if (!cmd) {
			ret = cmd(arg, &ev);
		}
		return ret;
	}
	rc = ((void **) (con->out._ctx._buf + 1))[ret - 1];
	ev.reply.context = rc;
	
	if (rc->len) {
		if (rc->_val[0] & 0x80) {
			MPT_STRUCT(command) *ans;
			uint64_t ansid;
			
			rc->_val[0] &= 0x7f;
			ret = mpt_message_buf2id(rc->_val, rc->len, &ansid);
			if (ret < 0 || ret > (int) sizeof(ev.id)) {
				connectionLog(con, __func__, MPT_FCNLOG(Error), "%s: %s (%d)",
				              MPT_tr("reply failed"), MPT_tr("bad id size"), ret);
				return MPT_ERROR(MissingBuffer);
			}
			if (!(ans = mpt_command_get(&con->_wait, ansid))) {
				connectionLog(con, __func__, MPT_FCNLOG(Error), "%s: %s ("PRIx64")",
				              MPT_tr("reply failed"), MPT_tr("message not registered"), ansid);
				return MPT_ERROR(MissingBuffer);
			}
			if (ans->cmd(ans->arg, &msg) < 0) {
				connectionLog(con, __func__, MPT_FCNLOG(Error), "%s: %s",
				              MPT_tr("dispatch failed"), MPT_tr("no message available"));
				return MPT_ERROR(MissingBuffer);
			}
			return 0;
		}
		if (!cmd) {
			replySet(rc, 0);
			return 0;
		}
		ev.reply.set = replySet;
	}
	ret = cmd(arg, &ev);
	
	if (ret < 0 && rc && rc->len) {
		replySet(rc, 0);
	}
	
	return ret;
}
