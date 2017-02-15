/*!
 * assign stream to existing file descriptor.
 */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/mman.h>

#include "queue.h"

#include "stream.h"

/*!
 * \ingroup mptStream
 * \brief set stream backing
 * 
 * Use socket descriptor to set stream backing descriptor.
 * 
 * \param stream stream descriptor
 * \param fd     socket descriptor
 * \param mode   options for buffering and mode
 * 
 * \retval -1  unable to set descriptor
 * \retval -2  unable to set mode
 * \retval >=0 effective mode
 */
extern int mpt_stream_dopen(MPT_STRUCT(stream) *stream, const MPT_STRUCT(socket) *fd, int mode)
{
	MPT_STRUCT(stream) tmp = MPT_STREAM_INIT;
	int fwrite, fread, fdflags = O_RDWR | O_WRONLY;
	
	fwrite = fread = fd->_id;
	
	/* get file param for valid descriptor */
	if ((fdflags = fcntl(fread, F_GETFL)) < 0) {
		return -1;
	}
	if (mode & MPT_STREAMFLAG(RdWr)) {
		if (!(fdflags & O_RDWR)) {
			errno = EBADF;
			return -1;
		}
	}
	/* set file parameter for writeable */
	else if (mode & MPT_STREAMFLAG(Write)) {
		if (!(fdflags & (O_WRONLY | O_RDWR))) {
			errno = EBADF;
			return -1;
		}
		mode &= ~MPT_STREAMFLAG(ReadBuf);
		fread = -1;
	}
	/* invalid file flags for "read only" */
	else if (!(fdflags & O_RDWR) && (fdflags & O_WRONLY)) {
		errno = EBADF;
		return -1;
	}
	/* set file parameter for readable */
	else {
		mode  &= ~MPT_STREAMFLAG(WriteBuf);
		fwrite = -1;
	}
	
	if (_mpt_stream_setfile(&tmp._info, fread, fwrite) < 0) {
		return -1;
	}
	if (mpt_stream_setmode(&tmp, mode & ~0x3) < 0) {
		return -2;
	}
	tmp._rd._dec = stream->_rd._dec;
	tmp._wd._enc = stream->_wd._enc;
	mpt_stream_close(stream);
	*stream = tmp;
	
	return mode;
}

