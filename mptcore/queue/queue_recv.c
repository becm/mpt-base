/*!
 * decode message in queue
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
		size_t done = qu->_state.data.pos;
		
		if (qu->_state.data.msg >= 0) {
			done += qu->_state.data.msg;
		} else {
			done += qu->_state.data.len;
		}
		if (done > len) {
			return MPT_ERROR(BadEncoding);
		}
		len -= done;
		
		if (qu->_state.data.msg >= 0) {
			qu->_state.data.pos = done;
			qu->_state.data.msg = qu->_state.data.len;
			qu->_state.data.len = len;
			mpt_queue_shift(qu);
			return 1;
		}
		qu->_state.data.pos = done;
		qu->_state.data.len = len;
		mpt_queue_shift(qu);
		return 0;
	}
	max = vectorSet(&qu->data, src);
	
	if (((res = qu->_dec(&qu->_state, src, max)) >= 0)) {
		mpt_queue_shift(qu);
		return qu->_state.data.msg >= 0 ? 1 : 0;
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
	qu->_state.data.pos += max;
	qu->_state.curr += max;
	
	/* retry with bigger prefix space */
	max = vectorSet(&qu->data, src);
	if ((res = qu->_dec(&qu->_state, src, max)) < 0) {
		return res;
	}
	mpt_queue_shift(qu);
	return qu->_state.data.msg >= 0 ? 1 : 0;
}
