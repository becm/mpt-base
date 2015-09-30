
#include <string.h>

#include <sys/uio.h>

#include "message.h"
#include "queue.h"

/*!
 * \ingroup mptQueue
 * \brief get next message type
 * 
 * Get length/start of next message on queue.
 * 
 * \param qu  source queue
 * \param dec data decoder
 * \param dst decoded data buffer
 * 
 * \return (decoded) message length
 */
extern ssize_t mpt_queue_peek(const MPT_STRUCT(queue) *qu, MPT_STRUCT(codestate) *info, MPT_TYPE(DataDecoder) dec, const struct iovec *dst)
{
	MPT_STRUCT(message) msg;
	struct iovec src;
	size_t off, len;
	ssize_t ret;
	
	if (!(len = qu->len)) {
		return -2;
	}
	off = info->done;
	if (len < off) {
		return -1;
	}
	/* message setup */
	if (MPT_queue_frag(qu)) {
		msg.base = ((uint8_t *) qu->base) + qu->off;
		msg.used = qu->max - qu->off;
		msg.clen = 1;
		msg.cont = &src;
		src.iov_base = qu->base;
		src.iov_len  = qu->len - msg.used;
	} else {
		msg.base = ((uint8_t *) qu->base) + qu->off;
		msg.used = qu->len;
		msg.clen = 0;
		msg.cont = 0;
	}
	off = info->done;
	if (off && (mpt_message_read(&msg, off, 0) < off)) {
		return -1;
	}
	
	/* final data available */
	if (!dec) {
		if (!dst) {
			return mpt_message_length(&msg);
		}
		return mpt_message_read(&msg, dst->iov_len, dst->iov_base);
	}
	src.iov_base = (void *) msg.base;
	src.iov_len  = msg.used;
	
	/* start peek for new data */
	info->done = 0;
	ret = dec(info, &src, 0);
	len = info->done;
	info->done = len + off;
	
	if (ret < 0 || !dst) {
		return ret;
	}
	/* consume additional done data */
	if (len && (len = mpt_message_read(&msg, len, 0) < len)) {
		return -1;
	}
	len = src.iov_len;
	if ((size_t) ret > len) {
		ret = len;
	}
	/* get data start and length */
	if ((size_t) ret > dst->iov_len) {
		ret = dst->iov_len;
	}
	(void) memcpy(dst->iov_base, msg.base, ret);
	
	return ret;
}
