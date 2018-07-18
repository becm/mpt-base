
#include <string.h>

#include <sys/uio.h>

#include "message.h"
#include "queue.h"

/*!
 * \ingroup mptQueue
 * \brief preview next message
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
	int ret;
	
	if (!(len = qu->data.len)) {
		return MPT_ERROR(MissingData);
	}
	off = qu->data.off;
	msg.base = ((uint8_t *) qu->data.base) + off;
	
	/* message setup */
	if (len > (qu->data.max - off)) {
		msg.used = qu->data.max - off;
		msg.clen = 1;
		msg.cont = &src;
		src.iov_base = qu->data.base;
		src.iov_len  = len - msg.used;
	} else {
		msg.used = len;
		msg.clen = 0;
		msg.cont = 0;
	}
	off = qu->_state.data.pos;
	
	/* final data available */
	if (!qu->_dec) {
		if ((ret = qu->_state.data.msg) >= 0) {
			off += ret;
		}
		if (!dst) {
			ret = len - off;
			return ret >= 0 ? ret : MPT_ERROR(MissingData);
		}
		if (off && (mpt_message_read(&msg, off, 0) < off)) {
			return MPT_ERROR(MissingData);
		}
		return mpt_message_read(&msg, max, dst);
	}
	/* work area reduces offset */
	len = qu->_state.curr;
	if (len < off) {
		off = len;
	}
	if (off && (mpt_message_read(&msg, off, 0) < off)) {
		return MPT_ERROR(MissingData);
	}
	src.iov_base = (void *) msg.base;
	src.iov_len  = msg.used;
	
	/* reduce wrap size */
	qu->_state.data.pos -= off;
	qu->_state.curr -= off;
	/* peek for new data */
	ret = qu->_dec(&qu->_state, &src, 0);
	len = qu->_state.data.pos;
	msg.base = (uint8_t *) msg.base + len;
	/* restore offsets */
	qu->_state.data.pos = len + off;
	qu->_state.curr += off;
	len = qu->_state.data.len;
	
	if (ret < 0 || !dst) {
		return len;
	}
	/* get data start and length */
	if (len > max) {
		len = max;
	}
	(void) memcpy(dst, msg.base, len);
	
	return len;
}
