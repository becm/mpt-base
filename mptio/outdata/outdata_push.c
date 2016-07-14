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
	
	/* use stream data */
	if (!MPT_socket_active(&od->sock)) {
		MPT_STRUCT(stream) *srm;
		
		ret = len;
		if (!(srm = od->_buf)) {
			return MPT_ERROR(BadArgument);
		}
		if ((ret = mpt_stream_push(srm, len, src)) < 0) {
			return ret;
		}
		if (len) {
			od->state |= MPT_ENUM(OutputActive);
		} else {
			if (srm) {
				mpt_stream_flush(srm);
			}
			od->state &= ~MPT_ENUM(OutputActive);
		}
		return ret;
	}
	/* existing new data */
	if (od->state & MPT_ENUM(OutputReceived)) {
		return MPT_ERROR(MessageInput);
	}
	if (len) {
		if (!(od->state & MPT_ENUM(OutputActive))) {
			if (!src) {
				if (len != 1) {
					return MPT_ERROR(BadOperation);
				}
				od->state &= ~MPT_ENUM(OutputActive);
				return 0;
			}
			if ((buf = od->_buf)) {
				buf->used = 0;
			}
		}
		if (!mpt_array_append((MPT_STRUCT(array) *) &od->_buf, len, src)) {
			return MPT_ERROR(BadOperation);
		}
		od->state |= MPT_ENUM(OutputActive);
		return len;
	}
	else {
		struct msghdr mhdr;
		struct iovec out;
		
		if (!(buf = od->_buf)) {
			len = 0;
		} else {
			len = buf->used;
		}
		/* destination address */
		mhdr.msg_name    = 0;
		mhdr.msg_namelen = 0;
		
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
