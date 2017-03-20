/*!
 * get empty queue segment
 */

#include "queue.h"

/*!
 * \ingroup mptQueue
 * \~english
 * \brief empty queue data
 * 
 * Get start address and sizes of
 * unused queue parts.
 * 
 * \param queue queue to query
 * \param low   aligned size of returned address
 * \param high  size starting at queue base address
 */
extern void *mpt_queue_empty(const MPT_STRUCT(queue) *queue, size_t *low, size_t *high)
{
	size_t space, add;
	uint8_t *start;
	
	/* size directly available */
	if (!(space = queue->max - queue->len)) {
		return 0;
	}
	/* upper and lower part are separated */
	if (space <= queue->off) {
		start = queue->base;
		/* empty space after data */
		if ((add = queue->max - queue->off)) {
			start -= add;
			add = 0;
		}
		start += queue->len;
	}
	/* contigous data -> space available after and before */
	else {
		add = queue->off;
		space -= add;
		start = ((uint8_t *) queue->base) + queue->off + queue->len;
	}
	/* base and length of empty parts */
	*low = space;
	if (high) {
		*high = add;
	}
	
	return start;
}

