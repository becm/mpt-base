/*!
 * new stream bound to notifier.
 */

#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <poll.h>

#include "convert.h"
#include "event.h"

#include "stream.h"
#include "notify.h"

/*!
 * \ingroup mptNotify
 * \brief new connection
 * 
 * Create input bound to notifier.
 * Deduce encoding from connection type:
 *   normal file/fifo -> command
 *   tagged file/fifo -> cobs
 *   stream socket    -> cobs+ID
 * 
 * \param no   notification descriptor
 * \param dest destination type:address
 * 
 * \return file descriptor
 */
extern int mpt_notify_connect(MPT_STRUCT(notify) *no, const char *dest)
{
	MPT_STRUCT(socket) sock = MPT_SOCKET_INIT;
	MPT_STRUCT(fdmode) mode;
	MPT_INTERFACE(input) *in;
	int len, flags, enc = MPT_ENUM(EncodingCobs);
	
	if (!no || !dest) {
		errno = EFAULT;
		return 0;
	}
	if ((len = mpt_mode_parse(&mode, dest)) < 0) {
		len = mpt_mode_parse(&mode, 0);
	}
	if ((flags = mpt_connect(&sock, dest + len, &mode)) < 0) {
		return flags;
	}
	/* require input stream */
	if (!(flags & MPT_SOCKETFLAG(Stream)) || !(flags & MPT_SOCKETFLAG(Read))) {
		(void) close(sock._id);
		errno = EINVAL;
		return -1;
	}
	if (mode.family >= 0) {
		/* 2byte ID for bidirectional only */
		len   = sizeof(uint16_t);
		flags = MPT_STREAMFLAG(RdWr) | MPT_STREAMFLAG(Buffer);
	}
	else {
		if (!len) enc = MPT_ENUM(EncodingCommand);
		len   = 0;
		flags = MPT_STREAMFLAG(Read) | MPT_STREAMFLAG(ReadBuf);
	}
	if (!(in = mpt_stream_input(&sock, flags, enc, len))) {
		close(sock._id);
		errno = EINVAL;
		return -1;
	}
	if (mpt_notify_add(no, POLLIN, in) < 0) {
		in->_vptr->ref.unref((void *) in);
		return -1;
	}
	return sock._id;
}
