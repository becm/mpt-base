/*!
 * write data to stream (and backing file).
 */

#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#include "convert.h"

#include "queue.h"
#include "stream.h"

static ssize_t flushPosition(MPT_STRUCT(stream) *stream, const uint8_t *data, size_t len)
{
	const uint8_t *pos;
	int flush;
	
	if ((flush = MPT_stream_flush(mpt_stream_flags(&stream->_info))) < 0) {
		return 0;
	}
	if ((flush == 0) && (pos = memchr(data, 0, len))) {
		return pos - data + 1;
	}
	if ((flush == MPT_ENUM(NewlineMac)) && (pos = memchr(data, '\r', len))) {
		return pos - data + 1;
	}
	if ((flush == MPT_ENUM(NewlineUnix)) && (pos = memchr(data, '\n', len))) {
		return pos - data + 1;
	}
	if (flush == MPT_ENUM(NewlineNet)) {
		size_t curr, max;
		if ((data[0] == '\n') && (max = stream->_wd.data.len)) {
			uint8_t last;
			mpt_queue_get(&stream->_wd.data, max - 1, 1, &last);
			
			if (last == '\r') {
				return 1;
			}
			--len;
			++data;
		}
		curr = 0;
		while ((pos = memchr(data, '\r', len))) {
			size_t part = ++pos - data;
			if (!(len -= part)) {
				return curr;
			}
			data  = pos;
			curr += part;
			if (pos[0] == '\n') {
				return curr + 1;
			}
		}
	}
	return 0;
}

/*!
 * \ingroup mptStream
 * \brief write to stream
 * 
 * Write data to stream descriptor.
 * 
 * Use zero for @part to reserve space for @count bytes.
 * 
 * \param stream stream descriptor
 * \param count  number of elements to write
 * \param data   data base position
 * \param part   atomic element size
 * 
 * \return number of elements written
 */
extern size_t mpt_stream_write(MPT_STRUCT(stream) *stream, size_t count, const void *data, size_t part)
{
	int flags, file;
	size_t tchunk = 0;
	
	if (!count) {
		return count;
	}
	if (stream->_wd._enc) {
		errno = EINVAL;
		return 0;
	}
	flags = mpt_stream_flags(&stream->_info);
	file = -1;
	
	/* reserve data on queue */
	if (!part) {
		if (count > (part = stream->_wd.data.max - stream->_wd.data.len)) {
			if (!(flags & MPT_ENUM(StreamWriteBuf))
			    || (flags & MPT_ENUM(StreamWriteMap))
			    || !mpt_queue_prepare(&stream->_wd.data, count)) {
				return 0;
			}
			part = stream->_wd.data.max - stream->_wd.data.len;
		}
		return part/count;
	}
	
	do {
		size_t left, curr;
		
		left = stream->_wd.data.max - stream->_wd.data.len;
		
		/* get available chunks from buffer */
		if ((curr = left / part)) {
			size_t take;
			/* reduce to needed size */
			if (curr > (count - tchunk)) {
				curr = count - tchunk;
			}
			tchunk += curr;
			curr *= part;
			
			/* fill with zeros */
			if (!data) {
				part = stream->_wd.data.len;
				mpt_qpush(&stream->_wd.data, curr, 0);
				
				if (MPT_stream_flush(flags) == 0) {
					mpt_stream_flush(stream);
				}
				continue;
			}
			while (curr && (take = flushPosition(stream, data, curr))) {
				mpt_qpush(&stream->_wd.data, take, data);
				stream->_wd._state.done = stream->_wd.data.len;
				data = ((char *) data) + take;
				curr -= take;
			}
			mpt_stream_flush(stream);
			
			/* add data to queue */
			mpt_qpush(&stream->_wd.data, curr, data);
			data = ((char *) data) + curr;
			continue;
		}
		/* force flush to make room for new data */
		if (file < 0 && (file = _mpt_stream_fwrite(&stream->_info)) < 0) {
			if (!tchunk) {
				errno = EBADF;
			}
			return tchunk;
		}
		(void) mpt_queue_save(&stream->_wd.data, file);
		stream->_wd._state.done = stream->_wd.data.len;
		
		left = stream->_wd.data.max - stream->_wd.data.len;
		
		if (!(flags & MPT_ENUM(StreamWriteBuf))) {
			ssize_t len;
			if (!data || part != 1 || stream->_wd.data.len) {
				if (!tchunk) {
					errno = ENOTSUP;
				}
				return tchunk;
			}
			if ((len = write(file, data, count)) < 0) {
				return 0;
			}
			return len;
		}
		if (left < part) {
			if (flags & MPT_ENUM(StreamWriteMap)) {
				if (!tchunk) errno = EBADF;
				break;
			}
			if (!mpt_queue_prepare(&stream->_wd.data, part)) {
				if (!tchunk) errno = EAGAIN;
				break;
			}
		}
	} while (tchunk < count);
	
	return tchunk;
}

