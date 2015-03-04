/*!
 * assign stream to file name.
 */

#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/stat.h>

#include "queue.h"
#include "stream.h"

/*!
 * \ingroup mptStream
 * \brief open stream descriptor
 * 
 * Set file for stream descriptor according
 * to mode string.
 * 
 * \param stream stream descriptor
 * \param pos    relative position in stream
 * \param mode   type of base position
 * 
 * \retval >=0 position in stream
 * \retval <0  error
 */
extern int mpt_stream_open(MPT_STRUCT(stream) *srm, const char *path, const char *pmod)
{
	MPT_STRUCT(stream) tmp = MPT_STREAM_INIT;
	MPT_STRUCT(fdmode) mode;
	int file, fin, fout;
	
	if (!path) {
		errno = EFAULT;
		return -1;
	}
	if (!pmod) {
		mpt_mode_parse(&mode, 0);
	}
	else if (((file = mpt_mode_parse(&mode, pmod)) <= 0) || mode.family >= 0) {
		errno = EINVAL;
		return file;
	}
	if ((file = open(path, mode.param.file.open, mode.param.file.perm)) < 0) {
		return file;
	}
	fout = fin = file;
	switch (mode.stream & (MPT_ENUM(StreamWrite) | (MPT_ENUM(StreamRdWr)))) {
	  case MPT_ENUM(StreamRead):  fout = -1; break;
	  case MPT_ENUM(StreamWrite): fin  = -1; break;
	  default:;
	}
	if (_mpt_stream_setfile(&tmp._info, fin, fout) < 0) {
		close(file);
		errno = EINVAL;
		return -1;
	}
	if ((file = mpt_stream_setmode(&tmp, mode.stream & ~0xf)) < 0) {
		return file;
	}
	tmp._info._fd |= mode.stream & MPT_ENUM(StreamFlushLine);
	mpt_stream_setnewline(&tmp._info, mode.lsep, MPT_ENUM(StreamRdWr));
	
	mpt_stream_close(srm);
	*srm = tmp;
	return mode.stream;
}

