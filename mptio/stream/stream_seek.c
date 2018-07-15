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
	static const MPT_STRUCT(decode_state) def_dec = MPT_DECODE_INIT;
	static const MPT_STRUCT(encode_state) def_enc = MPT_ENCODE_INIT;
	MPT_STRUCT(queue) *qu;
	off_t add;
	int flags, file = -1;
	
	flags = mpt_stream_flags(&stream->_info);
	
	if ((add = pos) < 0 && pos > 0) {
		return MPT_ERROR(BadValue);
	}
	add = 0;
	switch (flags & (MPT_STREAMFLAG(Write) | MPT_STREAMFLAG(RdWr))) {
	    case MPT_STREAMFLAG(Read):
		if (stream->_rd._dec) {
			return MPT_ERROR(BadArgument);
		}
		qu = &stream->_rd.data;
		if (mode == SEEK_CUR && (flags & MPT_STREAMFLAG(ReadBuf))) {
			pos -= qu->len;
			add = qu->off;
		}
		file = _mpt_stream_fread(&stream->_info);
		stream->_rd._state = def_dec;
		break;
	    case MPT_STREAMFLAG(Write):
		if (stream->_wd._enc) {
			return MPT_ERROR(BadArgument);
		}
		if (stream->_wd._state.scratch) {
			return MPT_ERROR(BadOperation);
		}
		mpt_stream_flush(stream);
		qu = &stream->_wd.data;
		if (mode == SEEK_CUR && (flags & MPT_STREAMFLAG(WriteBuf))) {
			pos += qu->len;
		}
		file = _mpt_stream_fread(&stream->_info);
		stream->_wd._state = def_enc;
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
