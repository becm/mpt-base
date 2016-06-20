
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
 * 
 * \return (decoded) message length
 */
extern ssize_t mpt_queue_peek(MPT_STRUCT(decode_queue) *qu, size_t max, void *dst)
{
	MPT_STRUCT(message) msg;
	struct iovec src;
	size_t off, len;
	ssize_t ret;
	
	if (!(len = qu->data.len)) {
		return MPT_ERROR(MissingData);
	}
	off = qu->data.off;
	msg.base = ((uint8_t *) qu->data.base) + off;
	
	/* message setup */
	if (len > off) {
		msg.used = len - off;
		msg.clen = 1;
		msg.cont = &src;
		src.iov_base = qu->data.base;
		src.iov_len  = len - msg.used;
	} else {
		msg.used = len;
		msg.clen = 0;
		msg.cont = 0;
	}
	off = qu->_state.done;
	if (off && (mpt_message_read(&msg, off, 0) < off)) {
		return -1;
	}
	
	/* final data available */
	if (!qu->_dec) {
		if (!dst) {
			return mpt_message_length(&msg);
		}
		return mpt_message_read(&msg, max, dst);
	}
	src.iov_base = (void *) msg.base;
	src.iov_len  = msg.used;
	
	/* start peek for new data */
	qu->_state.done = 0;
	ret = qu->_dec(&qu->_state, &src, 0);
	len = qu->_state.done;
	msg.base = (uint8_t *) msg.base + len;
	qu->_state.done = len + off;
	
	if (ret < 0 || !dst) {
		return ret;
	}
	/* get data start and length */
	if ((size_t) ret > max) {
		ret = max;
	}
	(void) memcpy(dst, msg.base, ret);
	
	return ret;
}
