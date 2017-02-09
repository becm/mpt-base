/*!
 * stream controller and creator.
 */

/* request format definitions */
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "event.h"
#include "message.h"
#include "convert.h"

#include "notify.h"
#include "stream.h"

struct streamInput {
	MPT_INTERFACE(input) _in;
	MPT_STRUCT(stream) data;
	MPT_STRUCT(array) ctx;
	uint8_t idlen;
};

static void streamUnref(MPT_INTERFACE(unrefable) *in)
{
	struct streamInput *srm = (void *) in;
	MPT_STRUCT(buffer) *buf;
	
	if ((buf = srm->ctx._buf)) {
		MPT_STRUCT(reply_context) **ctx = (void *) (buf+1);
		mpt_reply_clear(ctx, buf->used / sizeof(*ctx));
		mpt_array_clone(&srm->ctx, 0);
	}
	(void) mpt_stream_close(&srm->data);
	free(srm);
}
static int streamNext(MPT_INTERFACE(input) *in, int what)
{
	struct streamInput *srm = (void *) in;
	return mpt_stream_poll(&srm->data, what, -1);
}
struct streamWrap
{
	struct streamInput *in;
	MPT_TYPE(EventHandler) cmd;
	void *arg;
};
static int streamReply(void *ptr, const MPT_STRUCT(message) *msg)
{
	static const char _func[] = "mpt::stream::reply";
	MPT_STRUCT(reply_context) *rp = ptr;
	struct streamInput *srm;
	
	if (!rp->used) {
		mpt_log(0, _func, MPT_LOG(Critical), "%s",
		        MPT_tr("reply context unregistered"));
		return MPT_REPLY(BadContext);
	}
	if (!(srm = rp->ptr)) {
		mpt_log(0, _func, MPT_LOG(Error), "%s: %s",
		        MPT_tr("unable to reply"), MPT_tr("output destroyed"));
		if (!--rp->used) {
			free(rp);
		}
		return MPT_REPLY(BadDescriptor);
	}
	if (!rp->len) {
		mpt_log(0, _func, MPT_LOG(Warning), "%s: %s",
		        MPT_tr("bad reply operation"), MPT_tr("reply already sent"));
		return MPT_REPLY(BadState);
	}
	if (mpt_stream_flags(&srm->data._info) & MPT_STREAMFLAG(MesgActive)) {
		mpt_log(0, _func, MPT_LOG(Warning), "%s: %s",
		        MPT_tr("unable to reply"), MPT_tr("message creation in progress"));
		return MPT_ERROR(MessageInProgress);
	}
	rp->val[0] |= 0x80;
	
	mpt_stream_push(&srm->data, rp->len, rp->val);
	mpt_stream_append(&srm->data, msg);
	mpt_stream_push(&srm->data, 0, 0);
	rp->len = 0;
	--rp->used;
	return 0;
}
static int streamMessage(void *ptr, const MPT_STRUCT(message) *msg)
{
	struct streamWrap *sw = ptr;
	struct streamInput *srm;
	MPT_STRUCT(event) ev;
	int idlen;
	
	srm = sw->in;
	
	ev.id = 0;
	ev.msg = msg;
	ev.reply.set = 0;
	ev.reply.context = 0;
	
	if (!(idlen = srm->idlen)) {
		return sw->cmd(sw->arg, &ev);
	}
	else {
		static const char _func[] = "mpt::stream::dispatch";
		MPT_STRUCT(reply_context) *rc;
		MPT_STRUCT(message) tmp;
		uint8_t id[UINT8_MAX];
		int i, ret;
		
		/* check id length limit */
		if (idlen > (int) sizeof(id)) {
			mpt_log(0, _func, MPT_LOG(Error), "%s: %s: %d",
			        MPT_tr("dispatch failed"), MPT_tr("message size too big"), idlen);
			return MPT_ERROR(BadArgument);
		}
		/* consume message ID */
		ev.msg = &tmp;
		tmp = *msg;
		if ((mpt_message_read(&tmp, idlen, id)) < (size_t) idlen) {
			mpt_log(0, _func, MPT_LOG(Error), "%s: %s",
			        MPT_tr("dispatch failed"), MPT_tr("message id incomplete"));
			return MPT_ERROR(BadValue);
		}
		/* reply indicated */
		if (id[0] & 0x80) {
			uint64_t rid;
			id[0] &= 0x7f;
			if ((ret = mpt_message_buf2id(id, idlen, &rid)) < 0) {
				mpt_log(0, _func, MPT_LOG(Error), "%s: %s",
				        MPT_tr("dispatch failed"), MPT_tr("message id incomplete"));
				return MPT_ERROR(BadValue);
			}
			if (ret > (int) sizeof(ev.id)) {
				mpt_log(0, _func, MPT_LOG(Error), "%s: %s (%08" PRIx64 ")",
				        MPT_tr("dispatch failed"), MPT_tr("message id too big"), rid);
				return MPT_ERROR(BadValue);
			}
			ev.id = rid;
			return sw->cmd(sw->arg, &ev);
		}
		/* unable to reply to message */
		if (!(mpt_stream_flags(&srm->data._info) & MPT_STREAMFLAG(Write))) {
			return sw->cmd(sw->arg, &ev);
		}
		for (i = 0; i < idlen; ++i) {
			/* skip zero elements */
			if (!id[i]) {
				continue;
			}
			/* reply context required */
			if (!(rc = mpt_reply_reserve(&srm->ctx, idlen))) {
				mpt_log(0, _func, MPT_LOG(Error), "%s: %s",
				        MPT_tr("dispatch failed"), MPT_tr("no reply context available"));
				return MPT_ERROR(BadOperation);
			}
			/* set reply context */
			memcpy(rc->val, id, idlen);
			rc->len = idlen;
			ev.reply.set = streamReply;
			ev.reply.context = rc;
			break;
		}
		ret = sw->cmd(sw->arg, &ev);
		
		/* generic reply to failed command */
		if (rc && ret < 0 && rc->len) {
			MPT_STRUCT(msgtype) hdr;
			hdr.cmd = MPT_ENUM(MessageAnswer);
			hdr.arg = ret;
			tmp.base = &hdr;
			tmp.used = sizeof(hdr);
			tmp.cont = 0;
			tmp.clen = 0;
			streamReply(rc, &tmp);
		}
		return ret;
	}
}
static int streamDispatch(MPT_INTERFACE(input) *in, MPT_TYPE(EventHandler) cmd, void *arg)
{
	struct streamInput *srm = (void *) in;
	struct streamWrap sw;
	
	if (srm->data._mlen < 0
	    && (srm->data._mlen = mpt_queue_recv(&srm->data._rd)) < 0) {
		if (_mpt_stream_fread(&srm->data._info) < 0) {
			return -2;
		}
		return 0;
	}
	if (!cmd) {
		srm->data._mlen = -1;
		return 1;
	}
	sw.in = srm;
	sw.cmd = cmd;
	sw.arg = arg;
	return mpt_stream_dispatch(&srm->data, streamMessage, &sw);
}
static int streamFile(MPT_INTERFACE(input) *in)
{
	struct streamInput *srm = (void *) in;
	return _mpt_stream_fread(&srm->data._info);
}

static const MPT_INTERFACE_VPTR(input) streamCtl = {
	{ streamUnref },
	streamNext,
	streamDispatch,
	streamFile
};

/*!
 * \ingroup mptStream
 * \brief create stream input
 * 
 * Create input from stream information.
 * 
 * \param from  stream descriptor to convert
 * \param code  coding of input stream
 * 
 * \return created input
 */
extern MPT_INTERFACE(input) *mpt_stream_input(const MPT_STRUCT(socket) *from, int mode, int code, size_t idlen)
{
	MPT_STRUCT(stream) tmp = MPT_STREAM_INIT;
	struct streamInput *srm;
	
	/* id size limit exceeded */
	if (idlen > UINT8_MAX) {
		errno = EINVAL;
		return 0;
	}
	/* id requires message format on stream */
	if (!code) {
		errno = EINVAL;
		return 0;
	}
	/* bidirectional mode for reply */
	if (mode & MPT_STREAMFLAG(Write)) {
		if (!(mode & MPT_STREAMFLAG(RdWr))) {
			errno = EINVAL;
			return 0;
		}
		if (!(tmp._wd._enc = mpt_message_encoder(code))) {
			return 0;
		}
	}
	/* input codec for message */
	if (!(tmp._rd._dec = mpt_message_decoder(code))) {
		return 0;
	}
	/* set temporary data, no resource leakage on error */
	if (mpt_stream_dopen(&tmp, from, mode) < 0) {
		errno = EINVAL;
		return 0;
	}
	if (!(srm = malloc(sizeof(*srm)))) {
		mpt_stream_close(&tmp);
		return 0;
	}
	srm->_in._vptr = &streamCtl;
	srm->data = tmp;
	srm->idlen = idlen;
	
	return &srm->_in;
}
