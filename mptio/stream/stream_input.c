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
#include "meta.h"
#include "message.h"
#include "convert.h"

#include "notify.h"
#include "stream.h"

MPT_STRUCT(streamInput) {
	MPT_INTERFACE(input) _in;
	MPT_STRUCT(stream) data;
	MPT_INTERFACE(reply_context) _rc;
	MPT_STRUCT(reply_data) rd;
};
/* reply context interface */
static int streamReply(MPT_INTERFACE(reply_context) *rc, const MPT_STRUCT(message) *msg)
{
	MPT_STRUCT(streamInput) *srm = MPT_baseaddr(streamInput, rc, _rc);
	static const char _func[] = "mpt::stream::reply";
	int ret;
	
	if (!srm->rd.len) {
		mpt_log(0, _func, MPT_LOG(Warning), "%s: %s",
		        MPT_tr("bad reply operation"), MPT_tr("reply already sent"));
		return MPT_ERROR(BadArgument);
	}
	ret = mpt_stream_reply(&srm->data, srm->rd.len, srm->rd.val, msg);
	
	if (ret >= 0) {
		srm->rd.len = 0;
	} else {
		uint64_t id = 0;
		mpt_message_buf2id(srm->rd.val, srm->rd.len, &id);
		mpt_log(0, _func, MPT_LOG(Warning), "%s: %s",
		        MPT_tr("bad reply operation"), MPT_tr("unable to send"));
		return MPT_ERROR(BadArgument);
	}
	return ret;
}
static MPT_INTERFACE(reply_context_detached) *streamDefer(MPT_INTERFACE(reply_context) *ctx)
{
	(void) ctx;
	return 0;
}
/* reference interface */
static void streamUnref(MPT_INTERFACE(reference) *ref)
{
	MPT_STRUCT(streamInput) *srm = (void *) ref;
	(void) mpt_stream_close(&srm->data);
	free(srm);
}
static uintptr_t streamRef(MPT_INTERFACE(reference) *ref)
{
	(void) ref;
	return 0;
}
/* metatype interface */
static int streamConv(const MPT_INTERFACE(metatype) *mt, int type, void *ptr)
{
	MPT_STRUCT(streamInput) *srm = (void *) mt;
	int me = mpt_input_type_identifier();
	
	if (!type) {
		if (ptr) {
			static const char fmt[] = { MPT_ENUM(TypeMeta), MPT_ENUM(TypeSocket), 0 };
			*((const char **) ptr) = fmt;
		}
		return me;
	}
	if (type == MPT_ENUM(TypeMeta)
	    || type == me) {
		if (ptr) {
			*((void **) ptr) = &srm->_in;
		}
		return MPT_ENUM(TypeSocket);
	}
	if (type == MPT_ENUM(TypeSocket)) {
		if (ptr) {
			*((int *) ptr) = _mpt_stream_fread(&srm->data._info);
		}
		return me;
	}
	return MPT_ERROR(BadType);
}
static MPT_INTERFACE(metatype) *streamClone(const MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
/* input interface */
static int streamNext(MPT_INTERFACE(input) *in, int what)
{
	MPT_STRUCT(streamInput) *srm = (void *) in;
	return mpt_stream_poll(&srm->data, what, -1);
}
struct streamWrap
{
	MPT_STRUCT(streamInput) *in;
	MPT_TYPE(EventHandler) cmd;
	void *arg;
};
static int streamMessage(void *ptr, const MPT_STRUCT(message) *msg)
{
	struct streamWrap *sw = ptr;
	MPT_STRUCT(streamInput) *srm;
	MPT_STRUCT(event) ev = MPT_EVENT_INIT;
	int idlen;
	
	srm = sw->in;
	
	ev.msg = msg;
	
	if (!(idlen = srm->rd._max)) {
		return sw->cmd(sw->arg, &ev);
	}
	else {
		static const char _func[] = "mpt::stream::dispatch";
		MPT_STRUCT(reply_context) *rc = 0;
		MPT_STRUCT(message) tmp;
		int i, ret;
		
		/* consume message ID */
		ev.msg = &tmp;
		tmp = *msg;
		if ((mpt_message_read(&tmp, idlen, srm->rd.val)) < (size_t) idlen) {
			mpt_log(0, _func, MPT_LOG(Error), "%s: %s",
			        MPT_tr("dispatch failed"), MPT_tr("message id incomplete"));
			return MPT_ERROR(BadValue);
		}
		/* reply indicated */
		if (srm->rd.val[0] & 0x80) {
			uint64_t rid;
			srm->rd.val[0] &= 0x7f;
			if ((ret = mpt_message_buf2id(srm->rd.val, idlen, &rid)) < 0) {
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
		if (!(MPT_stream_writable(mpt_stream_flags(&srm->data._info)))) {
			return sw->cmd(sw->arg, &ev);
		}
		for (i = 0; i < idlen; ++i) {
			/* skip zero elements */
			if (!srm->rd.val[i]) {
				continue;
			}
			srm->rd.len = idlen;
			ev.reply = rc = &srm->_rc;
			break;
		}
		ret = sw->cmd(sw->arg, &ev);
		
		/* generic reply to failed command */
		if (rc && srm->rd.len) {
			MPT_STRUCT(msgtype) hdr;
			hdr.cmd = MPT_MESGTYPE(Answer);
			hdr.arg = ret < 0 ? ret : 0;
			tmp.base = &hdr;
			tmp.used = sizeof(hdr);
			tmp.cont = 0;
			tmp.clen = 0;
			mpt_stream_reply(&srm->data, srm->rd.len, srm->rd.val, msg);
		}
		return ret;
	}
}
static int streamDispatch(MPT_INTERFACE(input) *in, MPT_TYPE(EventHandler) cmd, void *arg)
{
	MPT_STRUCT(streamInput) *srm = (void *) in;
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
	static const MPT_INTERFACE_VPTR(input) streamInput = {
		{ { streamUnref, streamRef },
		  streamConv,
		  streamClone
		},
		streamNext,
		streamDispatch
	};
	static const MPT_INTERFACE_VPTR(reply_context) streamContext = {
		streamReply,
		streamDefer
	};
	MPT_STRUCT(stream) tmp = MPT_STREAM_INIT;
	MPT_STRUCT(streamInput) *srm;
	
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
	if (!(srm = malloc(sizeof(*srm) + idlen))) {
		mpt_stream_close(&tmp);
		return 0;
	}
	srm->_in._vptr = &streamInput;
	srm->_rc._vptr = &streamContext;
	srm->data = tmp;
	srm->rd._max = idlen;
	srm->rd.len = 0;
	
	return &srm->_in;
}
