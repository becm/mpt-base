/*!
 * find element in queue.
 */

#include <errno.h>

#include "queue.h"

/*!
 * \ingroup mptQueue
 * \~english
 * \brief find element in queue
 * 
 * Match element in queue data.
 * 
 * \param queue queue to query
 * \param esz   element size
 * \param cmp   size starting at queue base address
 * \param carg  match compare argument
 * 
 * \return start of matched element
 */
extern void *mpt_queue_find(const MPT_STRUCT(queue) *queue, size_t esz, int (*cmp)(const void *, void *), void *carg)
{
	size_t pos, iter;
	char *addr;
	
	if (!queue || !cmp) {
		errno = EFAULT;
		return 0;
	}
	
	addr = queue->base;
	
	if (queue->len < esz) {
		errno = EAGAIN;
		return 0;
	}
	
	addr += queue->off;
	
	/* queue is aligned */
	if (!MPT_queue_frag(queue)) {
		iter = queue->len/esz;
	}
	else {
		/* elements in upper data part */
		iter = (queue->max - queue->off)/esz;
		
		for (pos = 0; pos < iter; pos++, addr += esz) {
			if (!cmp(addr, carg)) {
				return addr;
			}
		}
		iter = queue->max - queue->off;
		
		/* element wrapping over queue border */
		if (iter % esz) {
			errno = ENOTSUP;
			return 0;
		}
		/* remaining elements in lower data part */
		iter = (queue->len - iter)/esz;
		addr = queue->base;
	}
	
	for (pos = 0; pos < iter; pos++, addr += esz) {
		if (!cmp(addr, carg)) {
			return addr;
		}
	}
	
	return 0;
}

