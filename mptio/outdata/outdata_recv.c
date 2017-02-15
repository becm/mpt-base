/*!
 * finalize connection data
 */

#include <errno.h>

#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>

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
	MPT_STRUCT(buffer) *buf;
	struct iovec vec;
	uint8_t *addr;
	struct msghdr mh;
	int len, max, post;
	
	if (!MPT_socket_active(&out->sock)) {
		return MPT_ERROR(BadArgument);
	}
	if (out->state & MPT_OUTFLAG(Active)) {
		return MPT_ERROR(MessageInProgress);
	}
	out->state &= ~MPT_OUTFLAG(Received);
	
	/* UDP maxsize buffer */
#define MPT_MAX_UDP_SIZE 0x10000
	max = MPT_MAX_UDP_SIZE;
	if ((post = out->_idlen)) {
		post = out->_smax;
		max += post;
	}
	if (!(addr = mpt_array_slice(&out->buf, max, 0))) {
		return MPT_ERROR(BadOperation);
	}
	buf = out->buf._buf;
	
	vec.iov_base = addr;
	vec.iov_len  = buf->size;
	
	/* message default setup */
	mh.msg_name = 0;
	mh.msg_namelen = 0;
	mh.msg_iov = &vec;
	mh.msg_iovlen = 1;
	mh.msg_control = 0;
	mh.msg_controllen = 0;
	mh.msg_flags = 0;
	
	if ((mh.msg_namelen = post)) {
		mh.msg_name = addr + MPT_MAX_UDP_SIZE;
		vec.iov_len -= post;
	}
	/* get data from socket */
	if ((len = recvmsg(out->sock._id, &mh, 0)) < 0) {
		return MPT_ERROR(BadOperation);
	}
	if (len < out->_idlen) {
		return MPT_ERROR(MissingData);
	}
	buf->used = len;
	out->state |= MPT_OUTFLAG(Received);
	
	if ((out->_scurr = mh.msg_namelen)) {
		memmove(addr + len, addr + MPT_MAX_UDP_SIZE, mh.msg_namelen);
	}
	
	return post;
}
