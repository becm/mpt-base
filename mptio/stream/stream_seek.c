/*!
 * seek operations on MPT stream.
 */

#if defined(__FreeBSD__)
# define lseek64(f,p,m)  lseek(f,p,m)
#elif !defined(_LARGEFILE64_SOURCE)
# define _LARGEFILE64_SOURCE
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
	MPT_STRUCT(codestate) *state;
	off_t add = 0;
	int flags, file = -1;
	
	flags = mpt_stream_flags(&stream->_info);
	
	switch (flags & (MPT_ENUM(StreamWrite) | MPT_ENUM(StreamRdWr))) {
	    case MPT_ENUM(StreamRead):
		if (stream->_rd._dec) {
			return MPT_ERROR(BadArgument);
		}
		qu = &stream->_rd.data;
		state = &stream->_wd._state;
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
		qu = &stream->_wd.data;
		state = &stream->_wd._state;
		if (mode == SEEK_CUR && (flags & MPT_ENUM(StreamWriteBuf))) {
			pos += qu->len;
		}
		mpt_stream_flush(stream);
		file = _mpt_stream_fread(&stream->_info);
		break;
	    default:
		return MPT_ERROR(BadArgument);
	}
	if (file < 0) {
		return add;
	}
	if ((pos = lseek64(file, pos, mode)) < 0) {
		return MPT_ERROR(BadOperation);
	}
	qu->len = 0;
	qu->off = 0;
	
	state->done = 0;
	state->scratch = 0;
	
	return pos + add;
}
