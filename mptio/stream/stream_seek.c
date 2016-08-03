/*!
 * seek operations on MPT stream.
 */

#if defined(__linux__) && !defined(__x86_64__)
# if !defined(_LARGEFILE64_SOURCE)
#  define _LARGEFILE64_SOURCE
# endif
# define lseek(f,p,m) lseek64(f,p,m)
#endif

#include <stdio.h>
#include <unistd.h>

#include "queue.h"
#include "stream.h"

/*!
 * \ingroup mptStream
 * \brief stream seek operation
 * 
 * Seek in stream descriptor and backing file.
 * 
 * \param stream stream descriptor
 * \param pos    relative position in stream
 * \param mode   type of base position
 * 
 * \retval >=0 position in stream
 * \retval <0  error
 */
extern int64_t mpt_stream_seek(MPT_STRUCT(stream) *stream, int64_t pos, int mode)
{
	MPT_STRUCT(queue) *qu;
	off_t add;
	int flags, file = -1;
	
	flags = mpt_stream_flags(&stream->_info);
	
	if ((add = pos) < 0 && pos > 0) {
		return MPT_ERROR(BadValue);
	}
	add = 0;
	switch (flags & (MPT_ENUM(StreamWrite) | MPT_ENUM(StreamRdWr))) {
	    case MPT_ENUM(StreamRead):
		if (stream->_rd._dec) {
			return MPT_ERROR(BadArgument);
		}
		qu = &stream->_rd.data;
		stream->_wd._state.done = 0;
		stream->_wd._state.scratch = 0;
		if (mode == SEEK_CUR && (flags & MPT_ENUM(StreamReadBuf))) {
			pos -= qu->len;
			add = qu->off;
		}
		file = _mpt_stream_fread(&stream->_info);
		stream->_mlen = -2;
		break;
	    case MPT_ENUM(StreamWrite):
		if (stream->_wd._enc) {
			return MPT_ERROR(BadArgument);
		}
		mpt_stream_flush(stream);
		stream->_wd._state.done = 0;
		stream->_wd._state.scratch = 0;
		qu = &stream->_wd.data;
		if (mode == SEEK_CUR && (flags & MPT_ENUM(StreamWriteBuf))) {
			pos += qu->len;
		}
		file = _mpt_stream_fread(&stream->_info);
		break;
	    default:
		return MPT_ERROR(BadArgument);
	}
	if (file < 0) {
		return add;
	}
	if ((pos = lseek(file, pos, mode)) < 0) {
		return MPT_ERROR(BadOperation);
	}
	qu->len = 0;
	qu->off = 0;
	
	return pos + add;
}
