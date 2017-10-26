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

#include "meta.h"
#include "output.h"

#include "message.h"
#include "stream.h"

struct _streamWrapper
{
	MPT_STRUCT(connection) *con;
	MPT_TYPE(EventHandler) cmd;
	void *arg;
};
static int replyConnection(void *ptr, const MPT_STRUCT(reply_data) *rd, const MPT_STRUCT(message) *msg)
{
	MPT_STRUCT(connection) *con = ptr;
	
	if (!MPT_socket_active(&con->out.sock)) {
		MPT_STRUCT(buffer) *buf;
		
		if (!(buf = con->out.buf._buf)) {
			return MPT_ERROR(BadArgument);
		}
		if (con->out._idlen != rd->len) {
			return MPT_ERROR(BadValue);
		}
		return mpt_stream_reply((void *) buf, rd->len, rd->val, msg);
	}
	return mpt_outdata_reply(&con->out, rd->len, rd->val, msg);
}
int streamWrapper(void *ptr, const MPT_STRUCT(message) *msg)
{
	const struct _streamWrapper *wd = ptr;
	MPT_STRUCT(reply_data) *rd;
	MPT_STRUCT(reply_context) *rc;
	MPT_STRUCT(connection) *con;
	MPT_STRUCT(event) ev = MPT_EVENT_INIT;
	uint8_t idlen;
	
	con = wd->con;
	
	/* default to one-way processing */
	if (!(idlen = con->out._idlen)) {
		if (!wd->cmd) {
			return 0;
		}
		ev.msg = msg;
		return wd->cmd(wd->arg, &ev);
	}
	else {
		static const char _func[] = "mpt::connection::dispatch<mpt::stream>";
		MPT_STRUCT(message) tmp;
		uint64_t mid;
		uint8_t id[UINT8_MAX], i;
		MPT_STRUCT(metatype) *ctx;
		int ret;
		
		/* consume message ID */
		ev.msg = &tmp;
		tmp = *msg;
		if ((mpt_message_read(&tmp, idlen, id)) < idlen) {
			mpt_log(0, _func, MPT_LOG(Error), "%s: %s",
			        MPT_tr("dispatch failed"), MPT_tr("message id incomplete"));
			return MPT_ERROR(BadValue);
		}
		/* test reply indicator */
		if (id[0] & 0x80) {
			MPT_STRUCT(command) *ans;
			
			id[0] &= 0x7f;
			if ((ret = mpt_message_buf2id(id, idlen, &mid)) < 0
			    || ret > (int) sizeof(ans->id)) {
				mpt_log(0, _func, MPT_LOG(Error), "%s: %s",
				        MPT_tr("dispatch failed"), MPT_tr("reply id invalid"));
			}
			/* find reply handler */
			if (!(ans = mpt_command_get(&con->_wait, mid))) {
				mpt_log(0, _func, MPT_LOG(Error), "%s: %s (%08" PRIx64 ")",
				        MPT_tr("dispatch failed"), MPT_tr("unknown reply id"), mid);
				return MPT_ERROR(BadValue);
			}
			return ans->cmd(ans->arg, &tmp);
		}
		ctx = 0;
		for (i = 0; i < idlen; ++i) {
			/* skip zero elements */
			if (!id[i]) {
				continue;
			}
			/* reply context required */
			if ((ctx = con->_rctx)) {
				break;
			}
			if ((ctx = mpt_reply_deferrable(idlen, replyConnection, con))) {
				con->_rctx = ctx;
				break;
			}
			mpt_message_buf2id(id, idlen, &mid);
			mpt_log(0, _func, MPT_LOG(Warning), "%s: %s (%08" PRIx64 ")",
			        MPT_tr("dispatch incomplete"), MPT_tr("no context available"), mid);
			break;
		}
		/* get reply data and interface for reference */
		rc = 0;
		rd = 0;
		if (ctx
		    && ctx->_vptr->conv(ctx, MPT_ENUM(TypeReplyData), &rd) >= 0
		    && rd) {
			ctx->_vptr->conv(ctx, MPT_ENUM(TypeReply), &rc);
		}
		if ((ev.reply = rc) && mpt_reply_set(rd, idlen, id) < 0) {
			mpt_log(0, _func, MPT_LOG(Error), "%s: %s",
			        MPT_tr("dispatch failed"), MPT_tr("context not ready"));
			return MPT_ERROR(BadOperation);
		}
		if (!wd->cmd) {
			if (rc) {
				rc->_vptr->reply(rc, 0);
			}
			return 0;
		}
		ret = wd->cmd(wd->arg, &ev);
		/* generic reply to failed command */
		if (rc && rd->len) {
			MPT_STRUCT(msgtype) hdr;
			hdr.cmd = MPT_ENUM(MessageAnswer);
			hdr.arg = ret;
			tmp.base = &hdr;
			tmp.used = sizeof(hdr);
			tmp.cont = 0;
			tmp.clen = 0;
			rc->_vptr->reply(rc, &tmp);
		}
		return ret;
	}
}
/*!
 * \ingroup mptOutput
 * \brief dispatch data on connection
 * 
 * Decode and process messages on connection input.
 * 
 * \param con  connection descriptor
 * \param cmd  message to send
 */
extern int mpt_connection_dispatch(MPT_STRUCT(connection) *con, MPT_TYPE(EventHandler) cmd, void *arg)
{
	static const char _func[] = "mpt::connection::dispatch<mpt::output>";
	MPT_STRUCT(buffer) *buf;
	MPT_STRUCT(event) ev = MPT_EVENT_INIT;
	MPT_STRUCT(message) msg = MPT_MESSAGE_INIT;
	uint8_t *data;
	uint16_t hlen;
	uint8_t ilen, slen;
	
	/* message transfer in progress */
	if (con->out.state & MPT_OUTFLAG(Active)) {
		return MPT_EVENTFLAG(Retry);
	}
	if (!MPT_socket_active(&con->out.sock)) {
		struct _streamWrapper sw;
		
		if (!(buf = con->out.buf._buf)) {
			return MPT_ERROR(BadArgument);
		}
		return mpt_stream_dispatch((void *) buf, streamWrapper, &sw);
	}
	/* no new data present */
	if (!(con->out.state & MPT_OUTFLAG(Received))) {
		return MPT_EVENTFLAG(Retry);
	}
	con->out.state &= ~MPT_OUTFLAG(Received);
	
	/* no message data */
	if (!(buf = con->out.buf._buf)) {
		return cmd ? cmd(arg, &ev) : 0;
	}
	ilen = con->out._idlen;
	slen = con->out._smax;
	hlen = ilen + slen;
	
	if ((size_t) hlen > buf->_used) {
		mpt_log(0, _func, MPT_LOG(Error), "%s (%u < %u)", MPT_tr("datagram too small"), hlen, (int) buf->_used);
		mpt_outdata_reply(&con->out, slen, buf + 1, 0);
		return MPT_ERROR(BadValue);
	}
	/* discard existing message */
	if (!cmd) {
		buf->_used = 0;
		mpt_outdata_reply(&con->out, hlen, buf + 1, 0);
		return 0;
	}
	data = (void *) (buf + 1);
	/* no message id */
	if (!ilen) {
		MPT_STRUCT(message) msg = MPT_MESSAGE_INIT;
		msg.base = data + hlen;
		msg.used = buf->_used - hlen;
		ev.msg = &msg;
		return cmd(arg, &ev);
	}
	/* got reply message */
	if (data[0] & 0x80) {
		MPT_STRUCT(message) msg = MPT_MESSAGE_INIT;
		MPT_STRUCT(command) *ans;
		uint64_t id;
		int len;
		data[0] &= 0x7f;
		if ((len = mpt_message_buf2id(data + slen, ilen, &id)) < 0) {
			mpt_log(0, _func, MPT_LOG(Error), "%s (%i)",
			        MPT_tr("bad message length"), (int) ilen);
			buf->_used = slen;
			return MPT_ERROR(BadValue);
		}
		if (!(ans = mpt_command_get(&con->_wait, id))) {
			mpt_log(0, _func, MPT_LOG(Error), "%s: %s (" PRIx64 ")",
			        MPT_tr("reply processing failed"), MPT_tr("message not registered"), id);
			return MPT_ERROR(MissingBuffer);
		}
		msg.base = data + hlen;
		msg.used = buf->_used - hlen;
		if ((len = ans->cmd(ans->arg, &msg)) < 0) {
			mpt_log(0, _func, MPT_LOG(Error), "%s (%i)",
			        MPT_tr("reply processing failed"), len);
			return MPT_ERROR(MissingBuffer);
		}
		return 0;
	}
	else {
		MPT_INTERFACE(metatype) *ctx;
		MPT_INTERFACE(reply_context) *rc;
		MPT_STRUCT(reply_data) *rd;
		int ret;
		uint8_t i;
		
		/* need processing */
		for (i = 0; i < ilen; ++i) {
			if (!data[i]) {
				continue;
			}
			/* reply context required */
			if ((ctx = con->_rctx)) {
				break;
			}
			if (!(ctx = mpt_reply_deferrable(hlen, replyConnection, con))) {
				mpt_log(0, _func, MPT_LOG(Warning), "%s: %s",
				        MPT_tr("dispatch incomplete"), MPT_tr("no context available"));
				break;
			}
			con->_rctx = ctx;
			break;
		}
		/* get reply data and interface for reference */
		rc = 0;
		rd = 0;
		if (ctx
		    && ctx->_vptr->conv(ctx, MPT_ENUM(TypeReplyData), &rd) >= 0
		    && rd) {
			ctx->_vptr->conv(ctx, MPT_ENUM(TypeReply), &rc);
		}
		if ((ev.reply = rc) && mpt_reply_set(rd, ilen, data) < 0) {
			mpt_log(0, _func, MPT_LOG(Error), "%s: %s",
			        MPT_tr("dispatch failed"), MPT_tr("context not ready"));
			return MPT_ERROR(BadOperation);
		}
		msg.base = data + hlen;
		msg.used = buf->_used - hlen;
		ev.msg = &msg;
		
		ret = cmd(arg, &ev);
		
		if (rc && rd->len) {
			MPT_STRUCT(msgtype) hdr;
			hdr.cmd = MPT_ENUM(MessageAnswer);
			hdr.arg = ret;
			msg.base = &hdr;
			msg.used = sizeof(hdr);
			msg.cont = 0;
			rc->_vptr->reply(rc, &msg);
		}
		return ret;
	}
}
