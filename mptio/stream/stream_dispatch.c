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
extern int mpt_stream_dispatch(MPT_STRUCT(stream) *srm, size_t idlen, MPT_TYPE(EventHandler) cmd, void *arg)
{
	struct iovec vec;
	MPT_STRUCT(message) msg;
	MPT_STRUCT(event) ev;
	uint8_t id[32];
	int ret;
	
	/* require message separation */
	if (idlen) {
		if (!srm->_rd._dec) {
			return MPT_ERROR(BadArgument);
		}
		if (idlen > sizeof(id)) {
			return MPT_ERROR(BadValue);
		}
	}
	/* use existing or new message */
	if (srm->_mlen < 0
	    && (srm->_mlen = mpt_queue_recv(&srm->_rd)) < 0) {
		return -2;
	}
	/* remove message data from queue */
	mpt_queue_crop(&srm->_rd.data, 0, srm->_rd._state.done);
	srm->_rd._state.done = 0;
	
	/* get message data */
	mpt_message_get(&srm->_rd.data, 0, srm->_mlen, &msg, &vec);
	
	ev.id = 0;
	ev.msg = &msg;
	ev.reply.set = 0;
	ev.reply.context = srm;
	
	/* get message ID */
	if (idlen) {
		uint64_t evid;
		if (mpt_message_read(&msg, idlen, id) < idlen) {
			return MPT_ERROR(MissingData);
		}
		if (id[0] & 0x80) {
			id[0] &= 0x7f;
			ev.reply.context = 0;
		}
		if ((ret = mpt_message_buf2id(id, idlen, &evid)) < 0
		    || ret > (int) sizeof(ev.id)) {
			return MPT_ERROR(BadValue);
		}
		ev.id = evid;
	}
	/* consume message */
	if (!cmd) {
		/* reply to ignored message */
		if (idlen && ev.reply.context) {
			if (mpt_stream_flags(&srm->_info) & MPT_ENUM(StreamMesgAct)) {
				return MPT_ERROR(MessageInProgress);
			}
			mpt_stream_push(srm, idlen, id);
			mpt_stream_push(srm, 0, 0);
			mpt_stream_flush(srm);
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
