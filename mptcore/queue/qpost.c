/*!
 * prepare (uninitialized) element at queue end.
 */

#include <errno.h>

#include "queue.h"

/*!
 * \ingroup mptQueue
 * \brief prepare data at queue end
 * 
 * \param queue queue to reserve memory on
 * \param len   size of new element
 * 
 * \return elements left on queue
 */
extern ssize_t mpt_qpost(MPT_STRUCT(queue) *queue, size_t len)
{
	size_t low, high, total;
	
	/* try to fit in existing memory */
	if (!mpt_queue_empty(queue, &low, &high)) {
		return -2;
	}
	total = low + high;
	
	/* not enough remaining space */
	if (len > total) {
		errno = ERANGE;
		return 0;
	}
	total -= len;
	queue->len += len;
	
	return total / len;
}

