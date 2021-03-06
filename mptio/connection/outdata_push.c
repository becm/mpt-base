/*!
 * data output
 */

#include <stdio.h>
#include <string.h>

#include <sys/uio.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "message.h"
#include "output.h"

#include "stream.h"

#include "connection.h"

/*!
 * \ingroup mptConnection
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
	if (od->state & MPT_OUTFLAG(Received)) {
		return MPT_MESGERR(ActiveInput);
	}
	if (len) {
		/* reset outpu data */
		if (!src) {
			if (len != 1) {
				return MPT_ERROR(BadOperation);
			}
			if ((buf = od->buf._buf)) {
				buf->_used = 0;
			}
			od->state &= ~MPT_OUTFLAG(Active);
			return 0;
		}
		/* require target address */
		if (od->_smax && (!od->_scurr || !(buf = od->buf._buf) || buf->_used < od->_smax)) {
			return MPT_ERROR(BadValue);
		}
		/* force atomic message id start */
		if (od->_idlen && !(od->state & MPT_OUTFLAG(Active)) && len != od->_idlen) {
			return MPT_ERROR(BadValue);
		}
		/* new data to push */
		if (!mpt_array_append(&od->buf, len, src)) {
			return MPT_ERROR(BadOperation);
		}
		od->state |= MPT_OUTFLAG(Active);
		return len;
	}
	else {
		struct msghdr mhdr;
		struct iovec out;
		
		if (!(buf = od->buf._buf)) {
			len = 0;
		} else {
			len = buf->_used;
			buf->_used = 0;
		}
		if (!MPT_socket_active(&od->sock)) {
			od->state &= ~MPT_OUTFLAG(Active);
			return 0;
		}
		out.iov_base = buf+1;
		out.iov_len  = len;
		
		/* destination address */
		if ((mhdr.msg_namelen = od->_scurr)) {
			out.iov_base = ((uint8_t *) (buf+1)) + od->_smax;
			mhdr.msg_name = (void *) (buf+1);
		} else {
			mhdr.msg_name = 0;
		}
		/* set output */
		mhdr.msg_iov = &out;
		mhdr.msg_iovlen = 1;
		
		/* additional options */
		mhdr.msg_control = 0; mhdr.msg_controllen = 0;
		mhdr.msg_flags = 0;
		
		ret = sendmsg(od->sock._id, &mhdr, 0);
		
		if (ret >= 0) {
			od->state &= ~MPT_OUTFLAG(Active);
		}
		return ret;
	}
}
