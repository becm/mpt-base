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
	MPT_STRUCT(reply_context) *ctx = ctxp;
	MPT_STRUCT(stream) *srm;
	
	if (!(srm = ctx->ptr)) {
		mpt_log(0, "mpt::stream::reply", MPT_FCNLOG(Debug), "%s",
		        "source stream deleted");
		free(ctx);
		return 0;
	}
	if (!ctx->used) {
		mpt_log(0, "mpt::stream::reply", MPT_FCNLOG(Critical), "%s",
		        "called with unregistered context");
		return MPT_ERROR(MissingData);
	}
	if (mpt_stream_flags(&srm->_info) & MPT_ENUM(StreamMesgAct)) {
		return MPT_ERROR(MissingBuffer);
	}
	if (ctx->len) {
		mpt_stream_push(srm, ctx->len, ctx->_val);
	}
	/* detach context */
	ctx->ptr = 0;
	
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
extern int mpt_stream_dispatch(MPT_STRUCT(stream) *srm, MPT_STRUCT(reply_context) *ctx, MPT_TYPE(EventHandler) cmd, void *arg)
{
	struct iovec vec;
	MPT_STRUCT(message) msg;
	MPT_STRUCT(event) ev;
	ssize_t len;
	int ret;
	
	/* require message separation */
	if (ctx && !srm->_rd._dec) {
		return -1;
	}
	/* use existing or new message */
	if ((len = srm->_mlen) < 0
	    && (srm->_mlen = len = mpt_queue_recv(&srm->_rd)) < 0) {
		return -2;
	}
	/* remove message data from queue */
	mpt_queue_crop(&srm->_rd.data, 0, srm->_rd._state.done);
	srm->_rd._state.done = 0;
	
	/* get message data */
	mpt_message_get(&srm->_rd.data, 0, len, &msg, &vec);
	
	ev.id = 0;
	ev.msg = &msg;
	ev.reply.set = ctx ? sendMessage : 0;
	ev.reply.context = ctx;
	
	/* error to get message ID for reply context */
	if (ctx
	    && (len = ctx->len)
	    && (mpt_message_read(&msg, len, ctx->_val) < (size_t) len)) {
		return MPT_ERROR(BadValue);
	}
	/* consume message */
	if (!cmd) {
		/* reply to non-return message */
		if (ctx && len && ctx->_val[0] & 0x80) {
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
	/* further message on queue */
	if ((srm->_mlen = mpt_queue_recv(&srm->_rd)) >= 0) {
		ret |= MPT_ENUM(EventRetry);
	}
	return ret;
}
