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
	size_t cpos, wpos;
	int ret;
	
	/* use existing or new message */
	if (srm->_rd._state.content.len < 0
	    && (ret = mpt_queue_recv(&srm->_rd)) < 0) {
		return ret;
	}
	/* remove old data from queue */
	cpos = srm->_rd._state.content.pos;
	wpos = srm->_rd._state.work.pos;
	if ((wpos || srm->_rd._state.work.len) && wpos < cpos) {
		cpos = wpos;
	}
	mpt_queue_crop(&srm->_rd.data, 0, cpos);
	/* adapt state offset data */
	srm->_rd._state.content.pos -= cpos;
	srm->_rd._state.work.pos -= cpos;
	/* get message data */
	mpt_message_get(&srm->_rd.data, srm->_rd._state.content.pos, srm->_rd._state.content.len, &msg, &vec);
	
	/* consume message */
	if (!cmd) {
		ret = 0;
	}
	/* dispatch data to command */
	else if ((ret = cmd(arg, &msg)) < 0) {
		ret = MPT_EVENTFLAG(CtlError);
	}
	/* command handler succeded */
	else {
		ret &= MPT_EVENTFLAG(Flags);
	}
	/* further message on queue */
	if (mpt_queue_recv(&srm->_rd) >= 0) {
		ret |= MPT_EVENTFLAG(Retry);
	}
	return ret;
}
