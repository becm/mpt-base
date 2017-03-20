/*!
 * add/remove data on right side end.
 */

#include "queue.h"

extern int mpt_qpush(MPT_STRUCT(queue) *queue, size_t len, const void *data)
{
	int ret;
	if ((ret = mpt_qpost(queue, len)) < 0) {
		return ret;
	}
	/* set data in appended segment */
	return mpt_queue_set(queue, queue->len - len, len, data);
}

