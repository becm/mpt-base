
#include <string.h>
#include <errno.h>
#include <sys/uio.h>

#include "message.h"
#include "queue.h"

/*!
 * \ingroup mptQueue
 * \brief append data to queue
 * 
 * Encode and add data to queue.
 * 
 * Return value is size of data after queue length
 * to restore encoder state.
 * 
 * \param queue queue with encoded data and state
 * \param enc   encoder function
 * \param from  data to push
 * 
 * \return size of finished data
 */
extern ssize_t mpt_queue_push(MPT_STRUCT(encode_queue) *qu, size_t len, const void *base)
{
	struct iovec vec, from;
	ssize_t push;
	size_t low, high, done;
	uint8_t *dest;
	
	/* direct data append */
	if (!qu->_enc) {
		if (!len) {
			len = qu->data.len;
			qu->_state.done = len;
			qu->_state.scratch = 0;
			return len;
		}
		if (!base) {
			if (!qu->_state.scratch || len > 1) {
				return MPT_ERROR(BadOperation);
			}
			qu->data.len = qu->_state.done;
			qu->_state.scratch = 0;
			return 0;
		}
		done = qu->_state.done + qu->_state.scratch;
		if (done != qu->data.len) {
			return MPT_ERROR(BadEncoding);
		}
		low = qu->data.max - qu->data.len;
		if (!low) {
			return MPT_ERROR(MissingBuffer);
		}
		if (low > len) {
			low = len;
		}
		mpt_qpush(&qu->data, low, base);
		qu->_state.scratch += low;
		
		return low;
	}
	from.iov_base = (void *) base;
	from.iov_len  = len;
	dest = qu->data.base;
	
	/* clean aligned data */
	if (!(high = qu->data.off)) {
		vec.iov_base = dest;
		vec.iov_len  = low = qu->data.max;
		push = qu->_enc(&qu->_state, &vec, len ? &from : 0);
	}
	/* encode in upper part */
	else if ((done = qu->_state.done) >= (low = qu->data.max - high)) {
		vec.iov_base = dest + (done - low);
		vec.iov_len  = high;
		qu->_state.done -= low;
		push = qu->_enc(&qu->_state, &vec, len ? &from : 0);
		qu->_state.done += low;
	}
	/* start encoding in lower part */
	else {
		vec.iov_base = dest + high;
		vec.iov_len  = low;
		
		/* encoder data wrapping */
		if ((low - done) >= qu->_state.scratch) {
			push = qu->_enc(&qu->_state, &vec, len ? &from : 0);
			done = qu->_state.done;
		}
		/* exceed temporary buffer */
		else if (qu->_state.scratch >= 256) {
			push = -1;
		}
		/* try out-of-band wrapping */
		else {
			uint8_t buf[256];
			size_t max = qu->data.max - done;
			
			if (max > sizeof(buf)) {
				max = sizeof(buf);
			}
			mpt_queue_get(&qu->data, done, qu->_state.scratch, buf);
			vec.iov_base = buf;
			vec.iov_len  = max;
			
			qu->_state.done = 0;
			if ((push = qu->_enc(&qu->_state, &vec, len ? &from : 0)) >= 0) {
				size_t set = qu->_state.done + qu->_state.scratch;
				max = done + set;
				if (max > qu->data.max) {
					MPT_ABORT("invalid encoder state for queue");
				}
				qu->data.len = max;
				mpt_queue_set(&qu->data, done, set, buf);
			}
			done += qu->_state.done;
			qu->_state.done = done;
		}
		vec.iov_base = qu->data.base;
		
		/* bad encoding attempt */
		if (push < 0) {
			mpt_queue_align(&qu->data, 0);
			vec.iov_len = qu->data.max;
			push = qu->_enc(&qu->_state, &vec, len ? &from : 0);
		}
		/* incomlete append action */
		else if ((size_t) push < len) {
			ssize_t push2;
			
			from.iov_base = ((uint8_t *) base) + push;
			from.iov_len  = len - push;
			
			/* encode on alignd data */
			if (done < low) {
				mpt_queue_align(&qu->data, 0);
				vec.iov_len = qu->data.max;
				push2 = qu->_enc(&qu->_state, &vec, &from);
			}
			/* second push in upper part only */
			else {
				done -= low;
				qu->_state.done = done;
				vec.iov_len = high;
				push2 = qu->_enc(&qu->_state, &vec, &from);
				qu->_state.done += done;
			}
			if (push2 > 0) {
				push += push2;
			}
		}
	}
	/* correct queue range */
	len = qu->_state.done + qu->_state.scratch;
	qu->data.len = len;
	
	if (len > qu->data.max) {
		MPT_ABORT("invalid encoder state for queue");
	}
	return push;
}
