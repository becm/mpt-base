
#include "queue.h"

/*!
 * \ingroup mptQueue
 * \~english
 * \brief used queue data
 * 
 * Get start address and size of
 * first queue data part.
 * 
 * \param queue queue to query
 * \param low   size for returned segment
 */
extern void *mpt_queue_data(const MPT_STRUCT(queue) *queue, size_t *low)
{
	size_t start = queue->max - queue->off;
	uint8_t *base = ((uint8_t *) queue->base) + queue->off;
	
	/* require segment lengt pointer */
	if (start < queue->len) {
		if (!low) {
			return 0;
		}
		*low = start;
	}
	/* aligned queue data */
	else if (low) {
		*low = queue->len;
	}
	return base;
}
