/*!
 * add/remove data on right side end.
 */

#include <errno.h>

#include "queue.h"

extern int mpt_qpush(MPT_STRUCT(queue) *queue, size_t len, const void *data)
{
	if (mpt_qpost(queue, len) < 0) {
		return -2;
	}
	/* set data in new segment */
	return mpt_queue_set(queue, queue->len - len, len, data);
}

