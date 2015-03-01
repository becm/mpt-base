
#include <string.h>
#include <errno.h>

#include "queue.h"
/*!
 * \ingroup mptQueue
 * \brief crop queue data
 * 
 * Remove data segment from queue data.
 * 
 * \param queue queue to remove data from
 * \param off   position of data in queue
 * \param len   length of data to remove
 * 
 * \return operation result
 */
extern int mpt_queue_crop(MPT_STRUCT(queue) *queue, size_t pos, size_t len)
{
	size_t low, high, post;
	uint8_t *base;
	int ret = 0;
	
	base = mpt_queue_data(queue, &low);
	high = queue->len - low;
	
	/* remove from queue begin */
	if (!pos) {
		post = low + high;
		if (len > post) {
			errno = ERANGE;
			return -2;
		}
		queue->len -= len;
		if (len >= low) {
			len -= low;
			queue->off = len;
			return len ? 2 : 0;
		}
		queue->off += len;
		return 0;
	}
	/* start in lower part */
	if (pos < low) {
		low -= pos;
		base += pos;
	}
	/* start position out of range */
	else if ((pos -= low) > high) {
		errno = EINVAL;
		return -1;
	}
	/* process high part only */
	else {
		base = ((uint8_t *) queue->base) + pos;
		low = high - pos;
		high = 0;
	}
	post = low + high;
	
	if (post < len) {
		errno = ERANGE;
		return -2;
	}
	/* need to move post data */
	post -= len;
	
	/* move data over segments */
	if (high) {
		uint8_t *src = ((uint8_t *) queue->base) + len - low;
		if (low <= post) {
			memcpy(base, src, post);
			ret = 1;
		}
		else {
			/* limit moved data size */
			memcpy(base, src, low);
			post -= low;
			base = queue->base;
			/* start at offset 'low' in post data ((len - low) + low) */
			(void) memmove(base, base+len, post);
			ret = 3;
		}
	}
	/* linear data move */
	else if (post) {
		(void) memmove(base, base+len, post);
	}
	queue->len -= len;
	
	return ret;
}
