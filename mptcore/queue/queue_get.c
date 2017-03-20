/*!
 * get elemet (address/data) in queue range.
 * 
 * target pointer must be supplied id data is non-contigous.
 */

#include <string.h>

#include "queue.h"

extern int mpt_queue_get(const MPT_STRUCT(queue) *queue, size_t pos, size_t len, void *data)
{
	MPT_STRUCT(queue) tmp = *queue;
	size_t low, high;
	uint8_t *base;
	int ret = 0;
	
	if (!len) {
		return 0;
	}
	/* get segment properties */
	if (pos) {
		if (pos > (tmp.max - tmp.off)) {
			ret |= 1;
		}
		if (mpt_queue_crop(&tmp, 0, pos) < 0) {
			return MPT_ERROR(BadArgument);
		}
	}
	if (len > tmp.len) {
		return MPT_ERROR(BadArgument);
	}
	tmp.len = len;
	base = mpt_queue_data(&tmp, &low);
	high = tmp.len - low;
	
	if (!data) {
		if (high) ret |= 2;
		return ret;
	}
	/* copy data from queue, return base address on contigous data */
	if (low) {
		(void) memcpy(data, base, low);
	}
	if (high) {
		(void) memcpy(((uint8_t *) data) + low, queue->base, high);
		ret |= 2;
	}
	return ret;
}

