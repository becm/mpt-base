/*!
 * resize queue to specific size.
 */

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

#include "queue.h"

/*!
 * \ingroup mptQueue
 * \brief resize queue data
 * 
 * Allocate more memory for queue storage.
 * 
 * \param qu  source queue
 * \param len new data store size
 * 
 * \return start of new storage
 */
extern void *mpt_queue_resize(MPT_STRUCT(queue) *queue, size_t len)
{
	void *data;
	
	if (!len) {
		free(queue->base);
		queue->base = 0;
		queue->len = queue->off = queue->max = 0;
	}
	/* remove data from queue start */
	else if (len < queue->max) {
		if (len < queue->len) {
			mpt_queue_crop(queue, 0, queue->len - len);
		}
		mpt_queue_align(queue, 0);
		if (!(data = realloc(queue->base, len))) {
			return 0;
		}
		queue->base = data;
		queue->max  = len;
	}
	/* enlarge data area */
	else if (len > queue->max) {
		if (MPT_queue_frag(queue)) {
			mpt_queue_align(queue, 0);
		}
		if (!(data = realloc(queue->base, len))) {
			return 0;
		}
		queue->base = data;
		queue->max  = len;
	}
	return queue->base;
}

/*!
 * \ingroup mptQueue
 * \brief prepare queue data
 * 
 * Enshure availability on queue.
 * 
 * \param qu  source queue
 * \param len new data store size
 * 
 * \return remaining data on queue
 */
extern size_t mpt_queue_prepare(MPT_STRUCT(queue) *queue, size_t len)
{
	size_t left;
	
	if (len > (left = queue->max - queue->len)) {
		if ((SIZE_MAX-left) < len) {
			errno = EOVERFLOW;
			return 0;
		}
		len = (len - left) + queue->max;
		
		if (!mpt_queue_resize(queue, MPT_align(len))) {
			return 0;
		}
		left = queue->max - queue->len;
	}
	return left;
}
