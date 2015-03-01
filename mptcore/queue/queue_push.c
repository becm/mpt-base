
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
extern ssize_t mpt_queue_push(MPT_STRUCT(queue) *qu, MPT_TYPE(DataEncoder) enc, MPT_STRUCT(codestate) *info, struct iovec *from)
{
	struct iovec vec;
	ssize_t push;
	size_t low, high, len;
	uint8_t *dest;
	
	/* direct data append */
	if (!enc) {
		if (!from) {
			info->done = qu->len;
			info->scratch = 0;
			return qu->len;
		}
		if (!(high = from->iov_len)) {
			return -1;
		}
		if (!(dest = from->iov_base)) {
			if (!qu->len || from->iov_len > 1) {
				return -2;
			}
			qu->len = 0;
			return 0;
		}
		len = info->done + info->scratch;
		if (len != qu->len) {
			return -1;
		}
		low = qu->max - qu->len;
		if (low > high) {
			low = high;
		}
		mpt_qpush(qu, low, dest);
		from->iov_base = dest + low;
		from->iov_len -= low;
		
		info->scratch += low;
		
		return info->done;
	}
	dest = qu->base;
	
	/* clean aligned data */
	if (!(high = qu->off)) {
		vec.iov_base = dest;
		vec.iov_len  = low = qu->max;
		push = enc(info, &vec, from);
	}
	/* encode in upper part */
	else if ((len = info->done) >= (low = qu->max - high)) {
		vec.iov_base = dest;
		vec.iov_len  = high;
		info->done -= low;
		push = enc(info, &vec, from);
		info->done += low;
	}
	/* start encoding in lower part */
	else {
		vec.iov_base = dest + high;
		vec.iov_len  = low;
		
		/* encoder data wrapping */
		if ((low - len) >= info->scratch) {
			push = enc(info, &vec, from);
		}
		/* try out-of-band wrapping */
		else if (info->scratch < 256) {
			uint8_t buf[256];
			size_t max = qu->max - len;
			
			if (max > sizeof(buf)) {
				max = sizeof(buf);
			}
			mpt_queue_get(qu, len, info->scratch, buf);
			vec.iov_base = buf;
			vec.iov_len  = max;
			
			info->done = 0;
			if ((push = enc(info, &vec, from)) >= 0) {
				size_t set = info->done + info->scratch;
				max = len + set;
				if (max > qu->max) {
					MPT_ABORT("invalid encoder state for queue");
				}
				qu->len = max;
				mpt_queue_set(qu, len, set, buf);
			}
			info->done += len;
		}
		len = info->done;
		vec.iov_base = qu->base;
		
		/* bad encoding attempt */
		if (push < 0) {
			mpt_queue_align(qu, 0);
			vec.iov_len = qu->max;
			push = enc(info, &vec, from);
		}
		/* incomlete append action */
		else if (from && from->iov_base && from->iov_len && (from->iov_len -= push)) {
			from->iov_base = ((uint8_t *) from->iov_base) + push;
			
			if (len < low) {
				mpt_queue_align(qu, 0);
				vec.iov_len = qu->max;
				push = enc(info, &vec, from);
			}
			else {
				info->done  = len - low;
				vec.iov_len = high;
				push = enc(info, &vec, from);
				info->done += len - low;
			}
		}
	}
	/* correct data range */
	if (push >= 0 && from && from->iov_base && from->iov_len) {
		from->iov_base = ((uint8_t *) from->iov_base) + push;
		from->iov_len -= push;
		push = info->done;
	}
	len = info->done + info->scratch;
	
	if (len > qu->max) {
		MPT_ABORT("invalid encoder state for queue");
	}
	qu->len = len;
	
	return push;
}
