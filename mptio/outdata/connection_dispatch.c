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

struct _streamWrapper
{
	MPT_STRUCT(connection) *con;
	MPT_TYPE(EventHandler) cmd;
	void *arg;
};
int streamWrapper(void *ptr, const MPT_STRUCT(message) *msg)
{
	struct _streamWrapper *wd = ptr;
	MPT_STRUCT(connection) *con;
	MPT_STRUCT(event) ev = MPT_EVENT_INIT;
	uint8_t idlen;
	
	con = wd->con;
	
	/* default to one-way processing */
	if (!(idlen = con->out._idlen)) {
		return wd->cmd(wd->arg, &ev);
	}
	else {
		static const char _func[] = "mpt::connection::dispatch<mpt::stream>";
		MPT_STRUCT(message) tmp;
		MPT_STRUCT(reply_context) *rc = 0;
		MPT_STRUCT(reply_data) *rd;
		uint8_t id[UINT8_MAX], i;
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
			uint64_t rid;
			
			id[0] &= 0x7f;
			if ((ret = mpt_message_buf2id(id, idlen, &rid)) < 0
			    || ret > (int) sizeof(ans->id)) {
				mpt_log(0, _func, MPT_LOG(Error), "%s: %s",
				        MPT_tr("reply processing failed"), MPT_tr("message id invalid"));
			}
			/* find reply handler */
			if (!(ans = mpt_command_get(&con->_wait, rid))) {
				mpt_log(0, _func, MPT_LOG(Error), "%s: %s (%08"PRIx64")",
				        MPT_tr("reply processing failed"), MPT_tr("unknown id"), rid);
				return MPT_ERROR(BadValue);
			}
			return ans->cmd(ans->arg, &tmp);
		}
		for (i = 0; i < idlen; ++i) {
			/* skip zero elements */
			if (!id[i]) {
				continue;
			}
			/* reply context required */
			if (!(rc = con->_rctx)) {
				mpt_log(0, _func, MPT_LOG(Warning), "%s: %s",
				        MPT_tr("dispatch incomplete"), MPT_tr("no context available"));
				break;
			}
			break;
		}
		rd = (void *) (rc + 1);
		if ((ev.reply = rc) && mpt_reply_set(rd, idlen, id) < 0) {
			mpt_log(0, _func, MPT_LOG(Error), "%s: %s",
			        MPT_tr("dispatch failed"), MPT_tr("context not ready"));
			return MPT_ERROR(BadOperation);
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
			rc->_vptr->set(rc, &tmp);
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
	
	if ((size_t) hlen > buf->used) {
		mpt_log(0, _func, MPT_LOG(Error), "%s (%u < %u)", MPT_tr("datagram too small"), hlen, (int) buf->used);
		buf->used = slen;
		return MPT_ERROR(BadValue);
	}
	/* discard existing message */
	if (!cmd) {
		buf->used = slen;
		return 0;
	}
	data = (void *) (buf + 1);
	/* no message id */
	if (!ilen) {
		MPT_STRUCT(message) msg = MPT_MESSAGE_INIT;
		msg.base = data + hlen;
		msg.used = buf->used - hlen;
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
			buf->used = slen;
			return MPT_ERROR(BadValue);
		}
		if (!(ans = mpt_command_get(&con->_wait, id))) {
			mpt_log(0, _func, MPT_LOG(Error), "%s: %s ("PRIx64")",
			        MPT_tr("reply processing failed"), MPT_tr("message not registered"), id);
			return MPT_ERROR(MissingBuffer);
		}
		msg.base = data + hlen;
		msg.used = buf->used - hlen;
		if ((len = ans->cmd(ans->arg, &msg)) < 0) {
			mpt_log(0, _func, MPT_LOG(Error), "%s (%i)",
			        MPT_tr("reply processing failed"), len);
			return MPT_ERROR(MissingBuffer);
		}
		return 0;
	}
	else {
		MPT_STRUCT(reply_context) *rc = 0;
		MPT_STRUCT(reply_data) *rd;
		int ret;
		uint8_t i, scurr = con->out._scurr;
		
		memmove(data+scurr, data+slen, ilen);
		hlen = scurr + ilen;
		
		data += scurr;
		/* need processing */
		for (i = 0; i < ilen; ++i) {
			if (!data[i]) {
				continue;
			}
			/* reply context required */
			if (!(rc = con->_rctx)) {
				mpt_log(0, _func, MPT_LOG(Warning), "%s: %s",
				        MPT_tr("dispatch incomplete"), MPT_tr("no context available"));
			}
			break;
		}
		rd = (void *) (rc + 1);
		if ((ev.reply = rc) && mpt_reply_set(rd, ilen, data) < 0) {
			mpt_log(0, _func, MPT_LOG(Error), "%s: %s",
			        MPT_tr("dispatch failed"), MPT_tr("context not ready"));
			return MPT_ERROR(BadOperation);
		}
		msg.base = data + hlen;
		msg.used = buf->used - hlen;
		ev.msg = &msg;
		
		ret = cmd(arg, &ev);
		
		if (rc && rd->len) {
			MPT_STRUCT(msgtype) hdr;
			hdr.cmd = MPT_ENUM(MessageAnswer);
			hdr.arg = ret;
			msg.base = &hdr;
			msg.used = sizeof(hdr);
			msg.cont = 0;
			rc->_vptr->set(rc, &msg);
		}
		return ret;
	}
}
