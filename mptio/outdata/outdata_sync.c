/*!
 * finalize connection data
 */


#include <sys/uio.h>
#include <arpa/inet.h>

#include <poll.h>

#include "array.h"
#include "queue.h"
#include "event.h"
#include "message.h"

#include "stream.h"

#include "output.h"

/*!
 * \ingroup mptOutput
 * \brief syncronize connection
 * 
 * Wait for all sent messages to produce reply.
 * Interrupt (return -2) if non-reply message is received.
 * 
 * \param out      output data descriptor
 * \param timeout  time to wait for return messages
 * 
 * \return state of operation
 */
extern int mpt_outdata_sync(MPT_STRUCT(outdata) *out, const MPT_STRUCT(array) *wait, int timeout)
{
	struct pollfd sock;
	struct iovec vec;
	MPT_STRUCT(command) *ans;
	size_t len = 1;
	int ret;
	
	if (!out->_idlen) {
		return MPT_ERROR(BadArgument);
	}
	
	if (!MPT_socket_active(&out->sock)) {
		MPT_STRUCT(stream) *srm;
		if ((srm = out->_buf)) {
			return mpt_stream_sync(srm, out->_idlen, wait, timeout);
		}
		return MPT_ERROR(BadArgument);
	}
	if (out->state & MPT_ENUM(OutputActive)) {
		return MPT_ERROR(MessageInProgress);
	}
	sock.events = POLLIN;
	
	/* count waiting handlers */
	if (wait) {
		const MPT_STRUCT(buffer) *buf;
		if (!(buf = wait->_buf) || !(len = buf->used)) {
			return 0;
		}
		len /= sizeof(*ans);
		
		for (ret = 0, ans = (void *) (buf + 1); len; --len, ++ans) {
			if (ans->cmd) {
				++ret;
			}
		}
		if (!(len = ret)) {
			return 0;
		}
	}
	/* UDP maxsize buffer */
	vec.iov_len = 0x10000;
	if (!(vec.iov_base = mpt_array_slice((MPT_STRUCT(array) *) &out->_buf, vec.iov_len, 0))) {
		MPT_STRUCT(buffer) *buf;
		if (!(buf = out->_buf) || !(vec.iov_len = buf->size)) {
			return MPT_ERROR(BadOperation);
		}
		vec.iov_base = buf+1;
	}
	while (len) {
		MPT_STRUCT(message) msg;
		struct msghdr mh;
		ssize_t curr;
		uint64_t id;
		
		mh.msg_name = 0;
		mh.msg_namelen = 0;
		mh.msg_iov = &vec;
		mh.msg_iovlen = 1;
		mh.msg_control = 0;
		mh.msg_controllen = 0;
		mh.msg_flags = 0;
		
		/* wait for input on socket */
		if ((ret = poll(&sock, 1, timeout)) < 0) {
			return len;
		}
		/* check data from socket */
		if ((curr = recvmsg(out->sock._id, &mh, 0)) < 0) {
			return MPT_ERROR(BadOperation);
		}
		if (curr < out->_idlen) {
			return MPT_ERROR(MissingData);
		}
		if ((size_t) curr > vec.iov_len) {
			return MPT_ERROR(MissingBuffer);
		}
		if (!(*((uint8_t *) vec.iov_base) & 0x80)) {
			return MPT_ERROR(MessageInput);
		}
		if (mpt_message_buf2id(vec.iov_base, out->_idlen, &id) < 0) {
			return MPT_ERROR(BadType);
		}
		if (!wait) {
			return 0;
		}
		if (!(ans = mpt_command_get(wait, id))) {
			return MPT_ERROR(BadValue);
		}
		/* process reply and clear wait state */
		ret = ans->cmd(ans->arg, &msg);
		ans->cmd = 0;
		--len;
		if (ret < 0) {
			return MPT_ERROR(BadOperation);
		}
	}
	return 0;
}
