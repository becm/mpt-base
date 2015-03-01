/*!
 * \ingroup mptQueue
 * \~english
 * \brief queue data from file
 * 
 * Append data read from file to queue.
 * 
 * \param queue queue to save data in
 * \param file  file descriptor to read from
 * \param len   size to read, zero to fill all available data
 */

#include <errno.h>
#include <sys/uio.h>

#include "queue.h"

extern ssize_t mpt_queue_load(MPT_STRUCT(queue) *queue, int file, size_t len)
{
	struct iovec io[2];
	uint8_t *base;
	size_t low, high;
	ssize_t ret;
	
	/* get empty parts in buffer */
	if (!(base = mpt_queue_empty(queue, &low, &high)))
		return -2;
	
	/* fill available space */
	if (!len) {
		;
	}
	/* read only to aligned part */
	else if (len < low) {
		low = len; high = 0;
	}
	/* read to both parts */
	else if ((len -= low) < high) {
		high = len;
	}
	/* prepare vector data */
	io[0].iov_base = base;
	io[0].iov_len  = low;
	io[1].iov_base = queue->base;
	io[1].iov_len  = high;
	
	/* load queue buffer data from file */
	if ((ret = readv(file, io, high ? 2 : 1)) <= 0) {
		return ret;
	}
	
	queue->len += ret;
	
	return ret;
}

