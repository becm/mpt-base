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
	if (srm->_rd._state.data.msg < 0) {
		if ((ret = mpt_queue_recv(&srm->_rd)) < 0) {
			return ret;
		}
		if (!ret) {
			return MPT_EVENTFLAG(None);
		}
	}
	/* get message data */
	mpt_message_get(&srm->_rd.data, srm->_rd._state.data.pos, srm->_rd._state.data.msg, &msg, &vec);
	
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
	if (mpt_queue_recv(&srm->_rd) > 0) {
		ret |= MPT_EVENTFLAG(Retry);
	}
	return ret;
}
