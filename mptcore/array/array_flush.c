
#include <errno.h>
#include <unistd.h>
#include <sys/uio.h>
#include <string.h>

#include <sys/un.h>
#include <netinet/in.h>

#include "array.h"
#include "message.h"

#include "stream.h"

/*!
 * \ingroup mptArray
 * \brief flush array data
 * 
 * Write array data slice to configured output.
 * 
 * \param od   output file descriptor
 * \param arr  data array
 * \param dest receiver address for data
 */
extern ssize_t mpt_array_flush(int fd, const MPT_STRUCT(slice) *s, const struct sockaddr *dest, socklen_t dlen)
{
	MPT_STRUCT(buffer) *buf;
	uint8_t *base;
	ssize_t ret, len, post;
	
	/* clear message filter */
	if (fd < 0 || !s) {
		errno = EINVAL;
		return -1;
	}
	/* no data available */
	if (!(buf = s->_a._buf) || !(len = buf->used)) {
		return 0;
	}
	/* data base */
	base = (void *) (buf+1);
	len  = buf->used;
	
	/* range validity */
	if ((ret = s->_off) > len) {
		errno = ERANGE;
		return -1;
	}
	len -= ret;
	if ((post = s->_len) > len) {
		errno = ERANGE;
		return -1;
	}
	base += ret;
	
	/* send atomic message */
	if (dlen) {
		struct msghdr mhdr;
		struct iovec out;
		
		/* destination address */
		mhdr.msg_name    = (void *) dest;
		mhdr.msg_namelen = dest ? dlen : 0;
		
		/* set output */
		out.iov_base = base;
		out.iov_len  = len;
		mhdr.msg_iov = &out;
		mhdr.msg_iovlen = 1;
		
		/* additional options */
		mhdr.msg_control = 0; mhdr.msg_controllen = 0;
		mhdr.msg_flags = 0;
		
		return sendmsg(fd, &mhdr, 0);
	}
	/* write to file descriptor */
	return write(fd, base, len);
}
