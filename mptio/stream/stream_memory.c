/*!
 * assign stream to memory.
 */

#include <errno.h>

#include <sys/uio.h>

#include "queue.h"
#include "stream.h"

/*!
 * \ingroup mptStream
 * \brief set stream buffer
 * 
 * Use memory as stream buffers.
 * 
 * \param stream stream descriptor
 * \param in     data for reading
 * \param out    memory for writing
 * 
 * \retval -1  bad data pointers
 * \retval >=0 effective mode
 */
extern int mpt_stream_memory(MPT_STRUCT(stream) *stream, const struct iovec *in, const struct iovec *out)
{
	int mode = 0;
	
	if (!in && !out) {
		errno = EINVAL;
		return -1;
	}
	_mpt_stream_setfile(&stream->_info, -1, -1);
	mpt_stream_setmode(stream, 0);
	
	if (out) {
		stream->_wd.data.base = out->iov_base;
		stream->_wd.data.max  = out->iov_len;
		if (!in) {
			return MPT_ENUM(StreamWrite);
		}
		if (in == out
		    || ((in->iov_base == out->iov_base) && (in->iov_len == out->iov_len))) {
			stream->_info._fd |= MPT_ENUM(StreamRdWr);
			return MPT_ENUM(StreamRdWr);
		}
		mode |= MPT_ENUM(StreamWrite);
	}
	if (in) {
		stream->_rd.data.base = in->iov_base;
		stream->_rd.data.max  = stream->_rd.data.len = in->iov_len;
		mode |= MPT_ENUM(StreamRead);
	}
	return mode;
}

