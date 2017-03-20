/*!
 * set queue offset to specific value,
 */

#include <string.h>

#include "queue.h"

extern void mpt_queue_align(MPT_STRUCT(queue) *queue, size_t pos)
{
	uint8_t *addr;
	size_t pv;
	
	if (pos > queue->max) {
		return;
	}
	if (!queue->len) {
		queue->off = 0;
		return;
	}
	if (!(addr = queue->base)) {
		return;
	}
	
	/* align to buffer start */
	if (MPT_queue_frag(queue)) {
		/* ensure continous data */
		if ((pv = queue->max - queue->len)) {
			(void) memmove(addr+queue->off-pv, addr+queue->off, queue->max - queue->off);
			queue->off -= pv;
		}
		mpt_memrev(addr, queue->off, queue->len);
		
		if (pos == (queue->off = 0))
			return;
	}
	else if (pos == queue->off)
		return;
	
	/* keep single continous data block */
	if ((pv = queue->max - queue->len) >= pos) {
		(void) memmove(addr+pos, addr+queue->off, queue->len);
		queue->off = pos;
		return;
	}
	
	/* split block into upper and lower part */
	mpt_memrev(addr+queue->off, pv = pos-queue->off, queue->len);
	
	/* move lower part to buffer data start */
	if (queue->off)
		(void) memmove(addr, addr+queue->off, pv);
	
	pos = queue->max - (queue->len - pv);
	
	if (pos != (queue->off + pv))
		(void) memmove(addr+pos, addr+queue->off+pv, queue->len-pv);
	
	return;
}

