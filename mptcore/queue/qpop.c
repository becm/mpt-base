
#include <string.h>
#include <errno.h>

#include "queue.h"

/*!
 * \ingroup mptQueue
 * \brief queue end data
 * 
 * Remove data from queue end.
 * 
 * \param queue fifo data storage
 * \param len   size to remove from end
 * \param data  target address
 * 
 * \return pointer to removed data
 */
extern void *mpt_qpop(MPT_STRUCT(queue) *queue, size_t len, void *data)
{
	size_t low, high;
	uint8_t *base;
	
	/* get data contentent/offset at queue end */
	base = mpt_queue_data(queue, &low);
	high = queue->len - low;
	
	/* aligned queue */
	if (!high) {
		if (len > low) {
			errno = ERANGE;
			return 0;
		}
		if (data) {
			memcpy(data, base, len);
		}
	}
	/* data in sparate parts */
	else if (len > high) {
		if (!data) {
			errno = EINVAL;
			return 0;
		}
		len -= high;
		if (len > low) {
			errno = ERANGE;
			return 0;
		}
		base = ((uint8_t *) queue->base) + queue->max - len;
		memcpy(data, base, high);
		memcpy(((uint8_t *) data) + len, queue->base, high);
		
		base = data;
	}
	/* data in high part */
	else {
		high -= len;
		base = ((uint8_t *) queue->base) + high;
		if (data) {
			memcpy(data, base, len);
		}
	}
	queue->len -= len;
	
	return base;
}
