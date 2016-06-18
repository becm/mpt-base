/*!
 * stream message dispatching.
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/uio.h>

#include "message.h"
#include "convert.h"
#include "event.h"
#include "queue.h"

#include "stream.h"

static int sendMessage(void *ctxp, const MPT_STRUCT(message) *msg)
{
	MPT_STRUCT(stream_context) *ctx = ctxp;
	MPT_STRUCT(stream) *srm;
	
	if (!(srm = ctx->srm)) {
		free(ctx);
		return 0;
	}
	if (mpt_stream_flags(&srm->_info) & MPT_ENUM(StreamMesgAct)) {
		return MPT_ERROR(MissingBuffer);
	}
	if (ctx->len) {
		mpt_stream_push(srm, ctx->len, ctx->_val);
	}
	/* detach context */
	ctx->srm = 0;
	
	/* send message or termination */
	if (msg) {
		mpt_stream_send(srm, msg);
	} else {
		mpt_stream_push(srm, 0, 0);
	}
	mpt_stream_flush(srm);
	
	return 0;
}

/*!
 * \ingroup mptStream
 * \brief dispatch next message
 * 
 * Call message dispatcher with next message
 * in input queue.
 * 
 * \param srm   stream descriptor
 * \param idlen message identifier length
 * \param cmd   command handler
 * \param arg   argument for command handler
 * 
 * \return created input
 */
extern int mpt_stream_dispatch(MPT_STRUCT(stream) *srm, MPT_STRUCT(stream_context) *ctx, MPT_TYPE(EventHandler) cmd, void *arg)
{
	struct iovec vec;
	MPT_STRUCT(message) msg;
	MPT_STRUCT(event) ev;
	ssize_t len;
	size_t off;
	int ret;
	
	/* require message separation */
	if (ctx && !srm->_dec.fcn) {
		return -1;
	}
	/* use existing or new message */
	if ((len = srm->_dec.mlen) < 0
	    && (srm->_dec.mlen = len = mpt_queue_recv(&srm->_rd, &srm->_dec.info, srm->_dec.fcn)) < 0) {
		return -2;
	}
	off = srm->_dec.info.done;
	srm->_dec.info.done = 0;
	mpt_message_get(&srm->_rd, off, len, &msg, &vec);
	
	ev.id = 0;
	ev.msg = &msg;
	ev.reply.set = srm->_dec.fcn ? sendMessage : 0;
	ev.reply.context = ctx;
	
	/* reserve reply context */
	if (ctx) {
		ctx->srm = srm;
		
		if ((len = ctx->len)
		    && (mpt_message_read(&msg, len, ctx->_val) < (size_t) len)) {
			mpt_queue_crop(&srm->_rd, 0, off);
			srm->_dec.mlen = mpt_queue_recv(&srm->_rd, &srm->_dec.info, srm->_dec.fcn);
			return MPT_ERROR(BadValue);
		}
	}
	/* consume message */
	if (!cmd) {
		/* reply to non-return message */
		if (ctx && ctx->len && ctx->_val[0] & 0x80) {
			sendMessage(ctx, 0);
		}
		ret = 0;
	}
	/* dispatch data to command */
	else if ((ret = cmd(arg, &ev)) < 0) {
		ret = MPT_ENUM(EventCtlError);
	} else {
		ret &= MPT_ENUM(EventFlags);
	}
	/* remove message data from queue */
	mpt_queue_crop(&srm->_rd, 0, off);
	
	/* further message on queue */
	if ((srm->_dec.mlen = mpt_queue_recv(&srm->_rd, &srm->_dec.info, srm->_dec.fcn)) >= 0) {
		ret |= MPT_ENUM(EventRetry);
	}
	return ret;
}
