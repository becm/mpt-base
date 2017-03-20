/*!
 * save data from queue to file.
 */

#include <sys/uio.h>

#include "queue.h"

extern ssize_t mpt_queue_save(MPT_STRUCT(queue) *queue, int file)
{
	struct iovec io[2];
	ssize_t len;
	
	if (!(len = queue->len)) {
		return 0;
	}
	/* get used parts */
	if (!(io[0].iov_base = mpt_queue_data(queue, &io[0].iov_len))) {
		return MPT_ERROR(MissingData);
	}
	/* prepare upper data part */
	io[1].iov_base = queue->base;
	io[1].iov_len  = len = len - io[0].iov_len;
	
	/* write queue buffer data to file */
	if ((len = writev(file, io, len ? 2 : 1)) <= 0) {
		return len;
	}
	/* remove written data from queue */
	mpt_queue_crop(queue, 0, len);
	
	return len;
}

