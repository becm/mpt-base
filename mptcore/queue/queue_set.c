/*!
 * set data valid queue memory.
 */

#include <errno.h>
#include <string.h>

#include "queue.h"

extern int mpt_queue_set(const MPT_STRUCT(queue) *queue, size_t pos, size_t len, const void *data)
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
		if (pos > (tmp.max-tmp.off)) {
			ret |= 1;
		}
		if (mpt_queue_crop(&tmp, 0, pos) < 0) {
			errno = EINVAL;
			return -1;
		}
	}
	if (len > tmp.len) {
		errno = ERANGE;
		return -2;
	}
	tmp.len = len;
	base = mpt_queue_data(&tmp, &low);
	high = tmp.len - low;
	
	/* set queue data */
	if (low) {
		if (data) {
			(void) memcpy(base, data, low);
			data = ((uint8_t *) data) + low;
		} else {
			(void) memset(base, 0, low);
		}
	}
	if (high) {
		if (data) {
			(void) memcpy(queue->base, data, high);
		} else {
			(void) memset(queue->base, 0, high);
		}
		ret |= 2;
	}
	return ret;
}

