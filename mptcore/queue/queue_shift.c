/*!
 * remove unused queu data
 */

#include "queue.h"

/*!
 * \ingroup mptQueue
 * \brief remove unused queue data
 * 
 * Remove unreferenced data at queue start.
 * 
 * \param qu  decoding queue
 */
extern void mpt_queue_shift(MPT_STRUCT(decode_queue) *qu)
{
	size_t curr, pos, len;
	   
	if (!(curr = qu->_state.curr)) {
		return;
	}
	pos = qu->_state.data.pos;
	len = qu->_state.data.len;
	if (pos || len) {
		if (pos < curr) {
			if (!(curr = pos)) {
				return;
			}
			pos = 0;
		} else {
			pos -= curr;
		}
	}
	if (mpt_queue_crop(&qu->data, 0, curr) < 0) {
		return;
	}
	qu->_state.curr -= curr;
	qu->_state.data.pos = pos;
}
