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
 * assume zero terminated data is message.
 * 
 * 
 * \param qu   message source queue
 * \param dec  data decoder
 * \param info decoder state
 * 
 * \return size of message
 */
extern ssize_t mpt_queue_recv(const MPT_STRUCT(queue) *qu, MPT_STRUCT(codestate) *info, MPT_TYPE(DataDecoder) dec)
{
	struct iovec src[2];
	uint8_t *base;
	size_t pre, len, max;
	
	if (!info) {
		return -1;
	}
	if (!(len = qu->len)) {
		return -2;
	}
	/* get new data part */
	if (!dec) {
		pre = info->done + info->scratch;
		
		if (pre > len) {
			return -1;
		}
		len -= pre;
		info->done = pre;
		info->scratch = len;
		
		return len ? (ssize_t) len : -2;
	}
	pre = qu->off;
	max = qu->max;
	
	base = (uint8_t *) qu->base;
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
	return dec(info, src, max);
}
