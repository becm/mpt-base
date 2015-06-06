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
#include <errno.h>

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
	off_t add = 0;
	int flags, file = -1;
	
	flags = mpt_stream_flags(&stream->_info);
	
	switch (flags & (MPT_ENUM(StreamWrite) | MPT_ENUM(StreamRdWr))) {
	    case MPT_ENUM(StreamRead):
		if (mode == SEEK_CUR && (flags & (MPT_ENUM(StreamReadBuf) | MPT_ENUM(StreamReadMap)))) {
			pos -= stream->_rd.len; add = stream->_rd.off;
			
		}
		file = _mpt_stream_fread(&stream->_info);
		break;
	    case MPT_ENUM(StreamWrite):
		if (mode == SEEK_CUR && (flags & (MPT_ENUM(StreamWriteBuf) | MPT_ENUM(StreamWriteMap)))) {
			pos += stream->_wd.len; stream->_wd.len = stream->_wd.off = 0;
		}
		file = _mpt_stream_fread(&stream->_info);
		break;
	    default:
		errno = ENOTSUP; return -1;
	}
	if (file < 0) return add;
	
	if ((pos = lseek64(file, pos, mode)) < 0) return pos;
	
	return pos + add;
}
