
#include <string.h>
#include <errno.h>
#include <sys/uio.h>

#include <arpa/inet.h>

#include "convert.h"

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
	
	if (flags & MPT_STREAMFLAG(WriteMap)) {
		flags &= ~MPT_STREAMFLAG(WriteBuf);
	}
	
	while (1) {
		ssize_t post;
		
		/* error during data append */
		if (len) {
			post = mpt_queue_push(&stream->_wd, len, src);
		}
		/* terminate current message */
		else {
			if (!stream->_wd._enc) {
				const char *fmt;
				size_t add, rem;
				
				rem = stream->_wd.data.max - stream->_wd.data.len;
				fmt = mpt_newline_string(MPT_stream_newline_write(flags));
				add = fmt ? strlen(fmt) : 0;
				if (rem >= add) {
					mpt_queue_push(&stream->_wd, add, fmt);
					stream->_info._fd &= ~MPT_STREAMFLAG(MesgActive);
					return 0;
				}
				post = MPT_ERROR(MissingBuffer);
			}
			else if ((post = mpt_queue_push(&stream->_wd, 0, 0)) >= 0) {
				stream->_info._fd &= ~MPT_STREAMFLAG(MesgActive);
				return post;
			}
		}
		/* pushed some data */
		if (post >= 0) {
			stream->_info._fd |= MPT_STREAMFLAG(MesgActive);
			total += post;
			/* push operation finished */
			if (!(len -= post)) {
				return total;
			}
			src = ((uint8_t *) src) + post;
			continue;
		}
		/* incompatible error state */
		if (post != MPT_ERROR(MissingBuffer)) {
			return total ? total : post;
		}
		/* queue not resizable */
		if (!(flags & MPT_STREAMFLAG(WriteBuf))
		    || (flags & MPT_STREAMFLAG(WriteMap))) {
			return total ? total : MPT_ERROR(BadArgument);
		}
		/* append queue data */
		if (!mpt_queue_prepare(&stream->_wd.data, 256)) {
			return total ? total : MPT_ERROR(BadOperation);
		}
	}
}
