
#include <errno.h>

#include <sys/uio.h>

#include "message.h"
#include "queue.h"

/*!
 * \ingroup mptQueue
 * \~english
 * \brief set message data
 * 
 * Get message from queue by index data.
 * 
 * \param      qu  queue to query
 * \param      idx message index
 * \param[out] msg message data
 * \param[out] vec upper message data
 */
extern int mpt_message_get(const MPT_STRUCT(queue) *qu, const MPT_STRUCT(msgindex) *idx, MPT_STRUCT(message) *msg, struct iovec *vec)
{
	uint8_t *base;
	size_t low, high, len;
	
	base = mpt_queue_data(qu, &low);
	high = qu->len - low;
	
	if (idx->off < low) {
		base += idx->off;
		low  -= idx->off;
	}
	else {
		len = idx->off - low;
		base = qu->base + len;
		if (len > high) {
			errno = EINVAL;
			return -1;
		}
		low  = high - len;
		high = 0;
	}
	if (idx->len > (len = low + high)) {
		errno = ERANGE;
		return -2;
	}
	if (idx->len <= low) {
		msg->base = base;
		msg->used = idx->len;
		msg->clen = 0;
		return 0;
	}
	else if (!vec) {
		errno = EINVAL;
		return -3;
	}
	else {
		msg->base = base;
		msg->used = low;
		msg->clen = 1;
		msg->cont = vec;
		vec->iov_base = qu->base;
		vec->iov_len  = idx->len - low;
		return 1;
	}
}
