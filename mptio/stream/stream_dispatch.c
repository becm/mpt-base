/*!
 * stream message dispatching.
 */

#include <sys/uio.h>

#include "message.h"

#include "event.h"

#include "stream.h"

/*!
 * \ingroup mptStream
 * \brief dispatch next message
 * 
 * Call message dispatcher with next message
 * in input queue.
 * 
 * \param srm  stream descriptor
 * \param cmd  command handler
 * \param arg  argument for command handler
 * 
 * \return created input
 */
extern int mpt_stream_dispatch(MPT_STRUCT(stream) *srm, int (*cmd)(void *, const MPT_STRUCT(message) *), void *arg)
{
	struct iovec vec;
	MPT_STRUCT(message) msg;
	int ret;
	
	/* use existing or new message */
	if (srm->_mlen < 0
	    && (srm->_mlen = mpt_queue_recv(&srm->_rd)) < 0) {
		return -2;
	}
	/* remove old data from queue */
	mpt_queue_crop(&srm->_rd.data, 0, srm->_rd._state.done);
	srm->_rd._state.done = 0;
	
	/* get message data */
	mpt_message_get(&srm->_rd.data, 0, srm->_mlen, &msg, &vec);
	
	/* consume message */
	if (!cmd) {
		ret = 0;
	}
	/* dispatch data to command */
	else if ((ret = cmd(arg, &msg)) < 0) {
		ret = MPT_ENUM(EventCtlError);
	}
	/* command handler succeded */
	else {
		ret &= MPT_ENUM(EventFlags);
	}
	/* further message on queue */
	if ((srm->_mlen = mpt_queue_recv(&srm->_rd)) >= 0) {
		ret |= MPT_ENUM(EventRetry);
	}
	return ret;
}
