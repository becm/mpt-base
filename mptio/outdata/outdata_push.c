/*!
 * data output
 */

#include <stdio.h>
#include <string.h>

#include <sys/uio.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "array.h"
#include "message.h"

#include "stream.h"

#include "output.h"

/*!
 * \ingroup mptOutput
 * \brief push to outdata
 * 
 * Append to outdata with specific encoding.
 * Flush buffer data to outinfo for len = 0.
 * 
 * \param od  outdata descriptor
 * \param len length of data to append
 * \param src data base address
 * 
 * \return size of consumed text
 */
extern ssize_t mpt_outdata_push(MPT_STRUCT(outdata) *od, size_t len, const void *src)
{
	ssize_t ret;
	MPT_STRUCT(buffer) *buf;
	
	/* existing new data */
	if (od->state & MPT_ENUM(OutputReceived)) {
		return MPT_ERROR(MessageInput);
	}
	if (len) {
		/* reset outpu data */
		if (!src) {
			if (len != 1) {
				return MPT_ERROR(BadOperation);
			}
			if ((buf = od->buf._buf)) {
				buf->used = 0;
			}
			od->state &= ~MPT_ENUM(OutputActive);
			return 0;
		}
		/* force atomic message id start */
		if (od->_idlen && !(od->state & MPT_ENUM(OutputActive)) && len != od->_idlen) {
			return MPT_ERROR(BadValue);
		}
		/* new data to push */
		if (!mpt_array_append(&od->buf, len, src)) {
			return MPT_ERROR(BadOperation);
		}
		od->state |= MPT_ENUM(OutputActive);
		return len;
	}
	else {
		struct msghdr mhdr;
		struct iovec out;
		
		if (!(buf = od->buf._buf)) {
			len = 0;
		} else {
			len = buf->used;
			buf->used = 0;
		}
		
		if (!MPT_socket_active(&od->sock)) {
			return 0;
		}
		/* destination address */
		if (src) {
			mhdr.msg_name    = (void *) src;
			mhdr.msg_namelen = od->_scurr;
		} else {
			mhdr.msg_name    = 0;
			mhdr.msg_namelen = 0;
		}
		/* set output */
		out.iov_base = buf+1;
		out.iov_len  = len;
		mhdr.msg_iov = &out;
		mhdr.msg_iovlen = 1;
		
		/* additional options */
		mhdr.msg_control = 0; mhdr.msg_controllen = 0;
		mhdr.msg_flags = 0;
		
		ret = sendmsg(od->sock._id, &mhdr, 0);
		
		if (ret >= 0) {
			od->state &= ~MPT_ENUM(OutputActive);
		}
		return ret;
	}
}
