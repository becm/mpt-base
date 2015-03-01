/*!
 * flush stream write buffer.
 */

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

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
	int file;
	
	/* write data */
	if (!stream->_wd.len) {
		return 0;
	}
	file = mpt_stream_flags(&stream->_info);
	
	/* update memory mapped file */
	if ((file & MPT_ENUM(StreamWriteMap))
	    && !(file & MPT_ENUM(StreamReadBuf))) {
		uint8_t *base = stream->_wd.base;
		size_t len = stream->_wd.len;
		if (MPT_queue_frag(&stream->_wd)) {
			len = stream->_wd.max - stream->_wd.off;
			msync(base, stream->_rd.len - len, MS_ASYNC);
		}
		msync(base + stream->_wd.off, len, MS_ASYNC);
		mpt_queue_crop(&stream->_wd, 0, stream->_wd.len);
	}
	/* try to save all data to backing file */
	else if ((file = _mpt_stream_fwrite(&stream->_info)) >= 0) {
		if (!mpt_queue_save(&stream->_wd, file)) {
			return -2;
		}
		if (!stream->_wd.len) {
			return 0;
		}
	}
	/* remaining data */
	return 1;
}
