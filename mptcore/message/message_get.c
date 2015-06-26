
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
extern int mpt_message_get(const MPT_STRUCT(queue) *qu, size_t off, size_t take, MPT_STRUCT(message) *msg, struct iovec *vec)
{
	uint8_t *base;
	size_t low, high, len;
	
	base = mpt_queue_data(qu, &low);
	high = qu->len - low;
	
	if (off < low) {
		base += off;
		low  -= off;
	}
	else {
		len = off - low;
		base = ((uint8_t *) qu->base) + len;
		if (len > high) {
			errno = EINVAL;
			return -1;
		}
		low  = high - len;
		high = 0;
	}
	if (take > (len = low + high)) {
		errno = ERANGE;
		return -2;
	}
	if (take <= low) {
		msg->base = base;
		msg->used = take;
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
		vec->iov_len  = take - low;
		return 1;
	}
}
