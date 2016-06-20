/*!
 * flush stream write buffer.
 */

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <sys/uio.h>
#include <sys/mman.h>

#include "queue.h"
#include "stream.h"

/*!
 * \ingroup mptStream
 * \brief flush stream
 * 
 * Write waiting data to backing file.
 * 
 * \param stream stream descriptor
 * 
 * \retval 0  no remaining data
 * \retval -2 unable to flush data
 * \retval 1  remaining data in write buffer
 */
extern int mpt_stream_flush(MPT_STRUCT(stream) *stream)
{
	size_t len;
	int file;
	
	/* write data */
	len = stream->_wd._state.done;
	if (!len) {
		return stream->_wd.data.len ? 1 : 0;
	}
	file = mpt_stream_flags(&stream->_info);
	
	/* update memory mapped file */
	if ((file & MPT_ENUM(StreamWriteMap))
	    && !(file & MPT_ENUM(StreamReadBuf))) {
		uint8_t *base = stream->_wd.data.base;
		size_t off, low;
		
		off = stream->_wd.data.off;
		low = stream->_wd.data.max - off;
		if (len > low) {
			msync(base, len - low, MS_ASYNC);
		} else {
			low = len;
		}
		msync(base + off, low, MS_ASYNC);
	}
	/* try to save ready data to backing file */
	else if ((file = _mpt_stream_fwrite(&stream->_info)) >= 0) {
		struct iovec io[2];
		/* get used parts */
		if (!(io[0].iov_base = mpt_queue_data(&stream->_wd.data, &io[0].iov_len))) {
			return -2;
		}
		/* prepare upper data part */
		if (len > io[0].iov_len) {
			io[1].iov_base = stream->_wd.data.base;
			io[1].iov_len  = len = len - io[0].iov_len;
		} else {
			io[0].iov_len = len;
			len = 0;
		}
		/* write queue buffer data to file */
		if ((len = writev(file, io, len ? 2 : 1)) <= 0) {
			if (!len) {
				mpt_stream_seterror(&stream->_info, MPT_ENUM(ErrorFull));
				return 1;
			} else {
				mpt_stream_seterror(&stream->_info, MPT_ENUM(ErrorWrite));
			}
			return len;
		}
	}
	/* remove written data from queue */
	mpt_queue_crop(&stream->_wd.data, 0, len);
	stream->_wd._state.done -= len;
	
	/* remaining data */
	return stream->_wd.data.len ? 1 : 0;
}
