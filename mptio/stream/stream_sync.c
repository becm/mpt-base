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
extern int mpt_stream_sync(MPT_STRUCT(stream) *stream, size_t idlen, const MPT_STRUCT(array) *arr, int timeout)
{
	MPT_STRUCT(command) *cmd;
	int count, pos, sav, len = 0;
	
	/* waiting messages */
	if (arr && arr->_buf) {
		cmd = (void *) (arr->_buf + 1);
		len = arr->_buf->used / sizeof(*cmd);
	}
	if (!len) {
		return 0;
	}
	/* count waiting ids */
	count = pos = 0;
	while (pos < len) {
		if (cmd[pos++].cmd) ++count;
	}
	/* no input size -> return waiting */
	if (!idlen) {
		return count;
	}
	/* get message from stream */
	while (count) {
		MPT_STRUCT(command) *mc;
		MPT_STRUCT(message) msg;
		struct iovec vec;
		uint8_t buf[32];
		uint64_t id;
		int ret;
		
		if (idlen > sizeof(buf)) {
			return MPT_ERROR(BadArgument);
		}
		/* get message data */
		if (stream->_mlen < 0) {
			if ((ret = mpt_stream_poll(stream, POLLIN, timeout)) < 0) {
				return MPT_ERROR(BadOperation);
			}
			if (!ret) {
				return count;
			}
			if (timeout > 0) {
				timeout = 0;
			}
			if ((stream->_mlen = mpt_queue_recv(&stream->_rd)) < 0) {
				continue;
			}
		}
		/* remove processed data */
		mpt_queue_crop(&stream->_rd.data, 0, stream->_rd._state.done);
		stream->_rd._state.done = 0;
		
		/* initial message data */
		mpt_message_get(&stream->_rd.data, 0, stream->_mlen, &msg, &vec);
		
		/* consume/create message id */
		mpt_message_read(&msg, idlen, buf);
		/* no return type message */
		if (!(buf[0] & 0x80)) {
			return MPT_ERROR(MessageInput);
		}
		buf[0] &= 0x7f;
		ret = mpt_message_buf2id(buf, idlen, &id);
		
		if (ret < 0 || ret > (int) sizeof(uintptr_t)) {
			return MPT_ERROR(BadValue);
		}
		/* handle message (and deregister handler) */
		if ((mc = mpt_command_find(cmd, len, id))) {
			ret = mc->cmd(mc->arg, &msg);
			mc->cmd = 0;
			--count;
		}
		/* find fallback command */
		else if ((mc = mpt_command_find(cmd, len, 0))) {
			ret = mc->cmd(mc->arg, &msg);
		}
		else {
			continue;
		}
		if (ret < 0) {
			break;
		}
	}
	if (count > len/2) {
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

