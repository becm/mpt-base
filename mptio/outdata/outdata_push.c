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
	struct iovec vec;
	ssize_t total;
	
	/* answer in progress */
	if (od->state & MPT_ENUM(OutputRemote)) {
		return -3;
	}
	/* need encoder for stream output */
	if (MPT_socket_active(&od->sock)
	    && (od->_sflg & MPT_ENUM(SocketStream))
	    && !od->_enc.fcn) {
		return -4;
	}
	/* add data to array */
	if (len) {
		vec.iov_base = (void *) src;
		vec.iov_len  = len;
		
		if ((total = mpt_array_push(&od->_buf, &od->_enc.info, od->_enc.fcn, &vec)) < 0) {
			return total;
		}
		/* set active state */
		od->state |= MPT_ENUM(OutputActive);
	}
	else {
		MPT_STRUCT(buffer) *buf;
		MPT_STRUCT(slice) s;
		
		/* message termination */
		if ((total = mpt_array_push(&od->_buf, &od->_enc.info, od->_enc.fcn, 0)) < 0) {
			return total;
		}
		/* set inactive state */
		od->state &= ~MPT_ENUM(OutputActive);
		
		/* no data or connection */
		if (!MPT_socket_active(&od->sock) || !(len = od->_enc.info.done)) {
			return total;
		}
		s._len = len;
		len += od->_enc.info.scratch;
		
		/* bad data ranges */
		if (!(buf = od->_buf._buf) || (len > buf->used)) {
			if (od->_enc.fcn) od->_enc.fcn(&od->_enc.info, 0, 0);
			return -1;
		}
		s._off = buf->used - len;
		
		/* send completed data */
		s._a._buf = buf;
		if ((total = mpt_array_flush(od->sock._id, &s, 0, (od->_sflg & MPT_ENUM(SocketStream)) ? 0 : 1)) < 0) {
			return total;
		}
		
		od->_enc.info.done -= total;
		len = s._off + total;
		
		/* remove head data */
		if ((len > buf->size/4) && (len > buf->used/2)) {
			mpt_array_cut(&od->_buf, 0, len);
		}
	}
	return total;
}
