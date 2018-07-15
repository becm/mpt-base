/*!
 * receive message from queue
 */

#include <sys/uio.h>

#include "array.h"
#include "queue.h"
#include "message.h"

int vectorSet(const MPT_STRUCT(queue) *qu, struct iovec src[2])
{
	uint8_t *base;
	size_t pre, max, len;
	
	pre = qu->off;
	max = qu->max;
	len = qu->len;
	base = (uint8_t *) qu->base;
	
	/* unaligned message data */
	if ((pre + len) > max) {
		src[0].iov_base = base + pre;
		src[0].iov_len  = max - pre;
		src[1].iov_len  = len - src[0].iov_len;
		src[1].iov_base = base;
		return 2;
	}
	/* linear data */
	src[0].iov_base = base + pre;
	src[0].iov_len  = len;
	return 1;
}

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
extern int mpt_queue_recv(MPT_STRUCT(decode_queue) *qu)
{
	struct iovec src[2];
	size_t len, max;
	ssize_t res;
	
	if (!(len = qu->data.len)) {
		return MPT_ERROR(MissingData);
	}
	/* get new data part */
	if (!qu->_dec) {
		size_t done = qu->_state.content.pos;
		
		if (qu->_state.content.len >= 0) {
			done += qu->_state.content.len;
		}
		if (done > len) {
			return MPT_ERROR(BadEncoding);
		}
		len -= done;
		qu->_state.content.pos = done;
		qu->_state.content.len = len;
		
		return len ? (ssize_t) len : MPT_ERROR(MissingData);
	}
	max = vectorSet(&qu->data, src);
	
	if (((res = qu->_dec(&qu->_state, src, max)) >= 0)) {
		return qu->_state.content.len >= 0 ? 1 : 0;
	}
	if (res != MPT_ERROR(MissingBuffer)) {
		return res;
	}
	/* queue full */
	max = qu->data.max;
	if (len >= max) {
		return res;
	}
	/* enlarge scratch space */
	max = max - len;
	if (mpt_qpre(&qu->data, max) < 0) {
		return MPT_ERROR(MissingBuffer);
	}
	/* correct data area offsets */
	qu->_state.content.pos += max;
	qu->_state.work.pos += max;
	
	/* retry with bigger prefix space */
	max = vectorSet(&qu->data, src);
	if ((res = qu->_dec(&qu->_state, src, max)) < 0) {
		return res;
	}
	return 0;
}
