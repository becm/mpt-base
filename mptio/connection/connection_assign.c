/*!
 * MPT I/O library
 *   set socket for connection
 */

#include <stdlib.h>
#include <string.h>

#include <sys/types.h>   /* socklen_t */
#include <sys/socket.h>  /* getsockopt() */

#include <unistd.h>  /* dup() */

#include "output.h"

#include "stream.h"

#include "connection.h"

/*!
 * \ingroup mptConnection
 * \brief set connection target
 * 
 * Set datagram or stream to clone of socket argument.
 * 
 * \param con     connection descriptor
 * \param sockptr socket to clone
 */
extern int mpt_connection_assign(MPT_STRUCT(connection) *con, const MPT_STRUCT(socket) *sockptr)
{
	MPT_STRUCT(socket) sock = MPT_SOCKET_INIT;
	MPT_STRUCT(stream) *srm = 0;
	int ret, fd;
	socklen_t size;
	
	if (!sockptr || (fd = sockptr->_id) < 0) {
		mpt_connection_close(con);
		return 0;
	}
	if (con->out.state & MPT_OUTFLAG(Active)) {
		return MPT_ERROR(BadOperation);
	}
	if ((fd = dup(fd)) < 0) {
		return MPT_ERROR(BadOperation);
	}
	size = sizeof(ret);
	if (getsockopt(fd, SOL_SOCKET, SO_TYPE, &ret, &size) < 0) {
		(void) close(fd);
		return MPT_ERROR(BadOperation);
	}
	sock._id = fd;
	if (ret == SOCK_DGRAM) {
		mpt_connection_close(con);
		con->out.sock = sock;
		return 1;
	}
	if (ret != SOCK_STREAM) {
		(void) close(fd);
		return MPT_ERROR(BadValue);
	}
	if (!MPT_socket_active(&con->out.sock)) {
		srm = (void *) con->out.buf._buf;
	}
	if (srm) {
		MPT_STRUCT(stream) tmp = MPT_STREAM_INIT;
		ret = mpt_stream_dopen(&tmp, &sock, MPT_STREAMFLAG(RdWr) | MPT_STREAMFLAG(Buffer));
		if (ret < 0) {
			(void) close(fd);
			return ret;
		}
		if (!(srm = malloc(sizeof(*srm)))) {
			mpt_stream_close(&tmp);
			return MPT_ERROR(BadOperation);
		}
		mpt_connection_close(con);
		con->out.buf._buf = memcpy(srm, &tmp, sizeof(*srm));
		return 1;
	}
	ret = mpt_stream_dopen(srm, &sock, MPT_STREAMFLAG(RdWr) | MPT_STREAMFLAG(Buffer));
	if (ret < 0) {
		(void) close(fd);
		return ret;
	}
	return 2;
}
