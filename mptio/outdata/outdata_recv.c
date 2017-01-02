/*!
 * finalize connection data
 */

#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>

#include "array.h"
#include "message.h"

#include "stream.h"

#include "output.h"

/*!
 * \ingroup mptOutput
 * \brief get socket datagram
 * 
 * Save datagram source and id to reply context and
 * payload to data buffer.
 * 
 * \param out  output data descriptor
 * 
 * \return state of operation
 */
extern int mpt_outdata_recv(MPT_STRUCT(outdata) *out)
{
	MPT_STRUCT(reply_context) *ctx;
	struct iovec vec[2];
	struct msghdr mh;
	ssize_t curr;
	int ret = 0;
	
	if (!MPT_socket_active(&out->sock)) {
		return MPT_ERROR(BadArgument);
	}
	if (out->state & MPT_ENUM(OutputActive)) {
		return MPT_ERROR(MessageInProgress);
	}
	out->state &= ~MPT_ENUM(OutputReceived);
	
	curr = out->_idlen;
	/* UDP maxsize buffer */
#define MPT_MAX_UDP_SIZE 0x10000
	if (!(vec[0].iov_base = mpt_array_slice((MPT_STRUCT(array) *) &out->_buf, MPT_MAX_UDP_SIZE-curr, 0))) {
		return MPT_ERROR(BadOperation);
	}
	vec[0].iov_len = MPT_MAX_UDP_SIZE-curr;
	
	/* message default setup */
	mh.msg_name = 0;
	mh.msg_namelen = 0;
	mh.msg_iov = vec;
	mh.msg_iovlen = 1;
	mh.msg_control = 0;
	mh.msg_controllen = 0;
	mh.msg_flags = 0;
	
	ret = 0;
	if (!curr) {
		ctx = 0;
	}
	else {
		MPT_STRUCT(reply_context) **base;
		MPT_STRUCT(buffer) *buf;
		int len;
		
		curr = MPT_align(curr);
		if (!(ctx = mpt_reply_reserve(&out->_ctx, curr + out->_socklen))) {
			return MPT_ERROR(BadOperation);
		}
		buf = out->_ctx._buf;
		base = (void *) (buf + 1);
		len = buf->used / sizeof(*base);
		
		while (ret < len) {
			if (ctx == base[ret++]) {
				break;
			}
		}
		ctx->ptr = out;
		ctx->len = out->_idlen;
		
		vec[1] = vec[0];
		vec[0].iov_base = ctx->_val;
		vec[0].iov_len  = out->_idlen;
		mh.msg_iovlen = 2;
		
		if ((mh.msg_namelen = out->_socklen)) {
			mh.msg_name = ctx->_val + curr;
		}
	}
	/* check data from socket */
	if ((curr = recvmsg(out->sock._id, &mh, 0)) < 0) {
		if (ctx) {
			ctx->used = 0;
		}
		return MPT_ERROR(BadOperation);
	}
	if (curr < out->_idlen) {
		if (ctx) {
			ctx->used = 0;
		}
		return MPT_ERROR(MissingData);
	}
	((MPT_STRUCT(buffer) *) out->_buf)->used = curr - out->_idlen;
	out->state |= MPT_ENUM(OutputReceived);
	
	return ret;
}
