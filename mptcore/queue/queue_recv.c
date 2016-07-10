/*!
 * receive message from queue
 */

#include <errno.h>

#include <sys/uio.h>

#include "array.h"
#include "queue.h"
#include "message.h"

/*!
 * \ingroup mptQueue
 * \brief get next message
 * 
 * Receive next complete message waiting on queue.
 * 
 * Message starts at codestate::done.
 * 
 * If no decoder function is supplied
 * use current available data as message.
 * 
 * \param qu   message source queue
 * 
 * \return size of message
 */
extern ssize_t mpt_queue_recv(MPT_STRUCT(decode_queue) *qu)
{
	struct iovec src[2];
	uint8_t *base;
	size_t pre, len, max;
	
	if (!(len = qu->data.len)) {
		return -2;
	}
	/* get new data part */
	if (!qu->_dec) {
		pre = qu->_state.done + qu->_state.scratch;
		
		if (pre > len) {
			return MPT_ERROR(BadEncoding);
		}
		len -= pre;
		qu->_state.done = pre;
		qu->_state.scratch = len;
		
		return len ? (ssize_t) len : MPT_ERROR(MissingData);
	}
	pre = qu->data.off;
	max = qu->data.max;
	
	base = (uint8_t *) qu->data.base;
	/* unaligned message data */
	if ((pre + len) > max) {
		src[0].iov_base = base + pre;
		src[0].iov_len  = max - pre;
		src[1].iov_len  = len - src[0].iov_len;
		src[1].iov_base = base;
		max = 2;
	}
	/* linear data */
	else {
		src[0].iov_base = base + pre;
		src[0].iov_len  = len;
		max = 1;
	}
	return qu->_dec(&qu->_state, src, max);
}
