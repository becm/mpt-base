
#include <string.h>
#include <errno.h>
#include <sys/uio.h>

#include <arpa/inet.h>

#include "queue.h"
#include "stream.h"

/*!
 * \ingroup mptStream
 * \brief push message data to stream
 * 
 * Add raw data to output queue in encoded form.
 * 
 * \param stream stream data
 * \param len data length
 * \param src data to push
 * 
 * \return encoded data length
 */
extern ssize_t mpt_stream_push(MPT_STRUCT(stream) *stream, size_t len, const void *src)
{
	struct iovec from;
	int flags = mpt_stream_flags(&stream->_info);
	
	from.iov_base = (void *) src;
	from.iov_len  = len;
	
	if (flags & MPT_ENUM(StreamWriteMap)) {
		flags &= ~MPT_ENUM(StreamWriteBuf);
	}
	
	while (1) {
		ssize_t post, total;
		
		/* terminate current message */
		if (!len) {
			if ((post = mpt_queue_push(&stream->_wd, stream->_enc.fcn, &stream->_enc.info, 0)) >= 0) {
				if (!stream->_enc.fcn) stream->_enc.info.done = 0;
				stream->_info._fd &= ~MPT_ENUM(StreamMesgAct);
			}
			return post;
		}
		/* error during data append */
		post = mpt_queue_push(&stream->_wd, stream->_enc.fcn, &stream->_enc.info, &from);
		total = len - from.iov_len;
		
		if (post < 0 && total) {
			return total;
		}
		stream->_info._fd |= MPT_ENUM(StreamMesgAct);
		
		/* push operation finished */
		if (!from.iov_len) {
			return len;
		}
		/* queue not resizable */
		if (!(flags & MPT_ENUM(StreamWriteBuf))) {
			return total ? total : MPT_ERROR(BadArgument);
		}
		if (!mpt_queue_prepare(&stream->_wd, 256)) {
			return total ? total : -1;
		}
	}
	
	return len;
}
