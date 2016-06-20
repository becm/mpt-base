
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
	ssize_t total = 0;
	int flags = mpt_stream_flags(&stream->_info);
	
	if (flags & MPT_ENUM(StreamWriteMap)) {
		flags &= ~MPT_ENUM(StreamWriteBuf);
	}
	
	while (1) {
		ssize_t post;
		
		/* terminate current message */
		if (!len) {
			if ((post = mpt_queue_push(&stream->_wd, 0, 0)) >= 0) {
				stream->_info._fd &= ~MPT_ENUM(StreamMesgAct);
			}
			return post;
		}
		/* error during data append */
		post = mpt_queue_push(&stream->_wd, len, src);
		
		/* pushed some data */
		if (post >= 0) {
			stream->_info._fd |= MPT_ENUM(StreamMesgAct);
			total += post;
			/* push operation finished */
			if (!(len -= post)) {
				return total;
			}
			src = ((uint8_t *) src) + post;
			continue;
		}
		/* queue not resizable */
		if (!(flags & MPT_ENUM(StreamWriteBuf))) {
			return total ? total : MPT_ERROR(BadArgument);
		}
		if (!mpt_queue_prepare(&stream->_wd.data, 256)) {
			return total ? total : -1;
		}
	}
}
