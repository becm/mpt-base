/*!
 * send rescale message via stream.
 */

#include <errno.h>
#include <poll.h>
#include <sys/uio.h>
#include <arpa/inet.h>

#include "array.h"
#include "queue.h"
#include "event.h"
#include "message.h"

#include "stream.h"

/*!
 * \ingroup mptStream
 * \brief wait for return messages
 * 
 * Process answers for messages registered via
 * supplied array.
 * 
 * \param stream  stream descriptor
 * \param idlen   message id size
 * \param arr     array containing MPT command elements
 * \param timeout max wait time between messages
 * 
 * \retval 0  no remaining data
 * \retval -2 unable to flush data
 * \retval 1  remaining data in write buffer
 */
extern int mpt_stream_sync(MPT_STRUCT(stream) *stream, size_t idlen, MPT_STRUCT(array) *arr, int timeout)
{
	MPT_STRUCT(command) *cmd;
	size_t count, pos, sav, len = 0;
	
	/* waiting messages */
	if (arr && arr->_buf) {
		cmd = (void *) (arr->_buf + 1);
		len = arr->_buf->used / sizeof(*cmd);
	}
	if (!len) return 0;
	
	/* count waiting ids */
	count = pos = 0;
	while (pos < len) {
		int (*fcn)();
		if (!(fcn = cmd[pos++].cmd)) continue;
		++count;
	}
	pos = count;
	
	/* no input -> trim array later */
	if (!stream->_dec.fcn) {
		count = 0;
	}
	
	/* get message from stream */
	while (count) {
		MPT_STRUCT(command) *mc;
		uint8_t buf[128];
		uintptr_t id;
		
		struct iovec vec;
		MPT_STRUCT(message) msg;
		MPT_STRUCT(msgindex) idx;
		ssize_t raw;
		size_t i;
		int ret;
		
		if (idlen > sizeof(id)) {
			errno = EINVAL;
			return -1;
		}
		vec.iov_base = buf;
		vec.iov_len  = sizeof(buf);
		
		/* get handler for message */
		if ((raw = mpt_queue_peek(&stream->_rd, &stream->_dec.info, stream->_dec.fcn, &vec)) < 0) {
			if (!timeout) {
				return count;
			}
			if ((ret = mpt_stream_poll(stream, POLLIN, timeout)) < 0) {
				return ret;
			}
			if (!ret) {
				return count;
			}
			if (timeout > 0) {
				timeout = 0;
			}
			continue;
		}
		/* no return type message */
		if (!(buf[0] & 0x80)) {
			return count;
		}
		/* get message data */
		idx.off = idx.len = 0;
		if ((raw = mpt_queue_recv(&stream->_rd, &stream->_dec.info, stream->_dec.fcn)) < 0) {
			return count;
		}
		idx.len = raw;
		idx.off = raw = stream->_dec.info.done;
		/* initial message data */
		mpt_message_get(&stream->_rd, &idx, &msg, &vec);
		
		/* consume/create message id */
		mpt_message_read(&msg, idlen, buf);
		id = buf[0] & 0x7f;
		for (i = 1; i < idlen; ++i) {
			id += (id * 0x100) + buf[i];
		}
		/* find command */
		if (!(mc = mpt_command_find(cmd, len, id))) {
			if (!(mc = mpt_command_find(cmd, len, 0))) {
				mpt_queue_crop(&stream->_rd, 0, raw);
				return -4;
			}
			ret = mc->cmd(mc->arg, &msg);
		}
		/* handle message (and deregister handler) */
		else {
			ret = mc->cmd(mc->arg, &msg);
			mc->cmd = 0;
			if (ret > 0) {
				--count;
			}
		}
		mpt_queue_crop(&stream->_rd, 0, raw);
		
		if (ret < 0) {
			break;
		}
	}
	if (count < len/3) {
		return count;
	}
	/* compress waiting return commands */
	for (pos = 0, sav = 0; pos < len; ++pos) {
		if (!cmd[pos].cmd) {
			continue;
		}
		if (pos != sav) {
			cmd[sav] = cmd[pos];
		}
		++sav;
	}
	arr->_buf->used = sav * sizeof(*cmd);
	
	return sav;
}

