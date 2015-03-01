/*!
 * add/remove data on left side end.
 */

#include <string.h>
#include <stdint.h>
#include <errno.h>

#include "queue.h"

/*!
 * \ingroup mptQueue
 * \brief consume queue data
 * 
 * Remove data from queue start.
 * 
 * \param queue fifo data element
 * \param len   size to consume
 * \param data  target for data
 * 
 * \return consumed data
 */
extern void *mpt_qshift(MPT_STRUCT(queue) *queue, size_t len, void *data)
{
	size_t low;
	uint8_t *addr;
	
	addr = mpt_queue_data(queue, &low);
	
	if (len <= low) {
		if (data) {
			(void) memcpy(data, addr, len);
		}
	}
	/* need data from both segments */
	else {
		if (!data) {
			errno = EINVAL;
			return 0;
		}
		if (len > queue->len) {
			errno = ERANGE;
			return 0;
		}
		addr = memcpy(data, addr, low);
		(void) memcpy(addr+low, queue->base, len-low);
	}
	/* remove data area from queue start */
	mpt_queue_crop(queue, 0, len);
	
	return addr;
}
