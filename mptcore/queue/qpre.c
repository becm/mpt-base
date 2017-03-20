/*!
 * reserve segment before queue data
 */

#include <string.h>
#include <stdint.h>

#include "queue.h"

/*!
 * \ingroup mptQueue
 * \brief prepare (uninitialized) element before queue start
 * 
 * \param queue queue to reserve memory on
 * \param len   size of new element
 * 
 * \return start of newly reserved memory
 */
extern ssize_t mpt_qpre(MPT_STRUCT(queue) *queue, size_t len)
{
	size_t low, high, total;
	
	mpt_queue_empty(queue, &low, &high);
	total = low + high;
	
	/* not enough remaining space */
	if (len > total) {
		return MPT_ERROR(MissingBuffer);
	}
	/* data element wraps around upper border */
	if (high && high < len) {
		queue->off = queue->max - (len - high);
	}
	else {
		if (len < queue->off) queue->off -= len;
		else queue->off += queue->max - len;
	}
	total -= len;
	queue->len += len;
	
	return total / len;
}
