/*!
 * write data to stream (and backing file).
 */

#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

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
		size_t curr;
		if ((data[0] == '\n') && (stream->_wd.len)) {
			uint8_t last;
			mpt_queue_get(&stream->_wd, stream->_wd.len - 1, 1, &last);
			
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
	if (stream->_enc.fcn) {
		errno = EINVAL;
		return 0;
	}
	flags = mpt_stream_flags(&stream->_info);
	file = -1;
	
	/* no data resize */
	if (!(flags & (MPT_ENUM(StreamWriteBuf)))) {
		/* direct file access */
		if ((file = _mpt_stream_fwrite(&stream->_info)) >= 0) {
			ssize_t len;
			if (!data || part != 1) {
				errno = ENOTSUP; return 0;
			}
			if ((len = write(file, data, count)) < 0) {
				return 0;
			}
			return len;
		}
		/* prepared data is available */
		tchunk = stream->_wd.max - stream->_wd.len;
		if (!part) {
			return tchunk > count ? 0 : tchunk;
		}
		if (!(tchunk /= part)) {
			return 0;
		}
		/* contigous growth only */
		if (flags & MPT_ENUM(StreamWriteMap)) {
			uint8_t * dest = stream->_wd.base;
			dest += stream->_wd.off;
			count = tchunk * part;
			if (data) {
				size_t flush = flushPosition(stream, data, count);
				memcpy(dest, data, count);
				if (flush) {
					stream->_wd.len += flush;
					mpt_stream_flush(stream);
					count -= flush;
				}
				stream->_wd.len += count;
			} else {
				memset(dest, 0, count);
				stream->_wd.len += count;
				if (MPT_stream_flush(flags) == 0) {
					mpt_stream_flush(stream);
				}
			}
			return tchunk;
		}
	}
	/* prepare write buffer */
	else if (part == 0) {
		if (count > (part = stream->_wd.max - stream->_wd.len)) {
			if (flags & MPT_ENUM(StreamWriteMap) || !mpt_queue_prepare(&stream->_wd, count)) {
				return 0;
			}
			part = stream->_wd.max - stream->_wd.len;
		}
		return part;
	}
	/* buffer bypass */
	else if (data && part == 1 && !stream->_wd.len && (file = _mpt_stream_fwrite(&stream->_info)) >= 0) {
		ssize_t len;
		if ((len = write(file, data, count)) > 0) {
			return len;
		}
	}
	/* disable resize for private mmap */
	else if (flags & MPT_ENUM(StreamWriteMap)) {
		flags &= ~MPT_ENUM(StreamWriteBuf);
	}
	
	do {
		size_t left, curr;
		
		left = stream->_wd.max - stream->_wd.len;
		
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
				part = stream->_wd.len;
				mpt_qpush(&stream->_wd, curr, 0);
				
				if (MPT_stream_flush(flags) == 0) {
					mpt_stream_flush(stream);
				}
				continue;
			}
			left = 0;
			while (curr && (take = flushPosition(stream, data, curr))) {
				left += take;
				mpt_qpush(&stream->_wd, take, data);
				data = ((char *) data) + take;
				curr -= take;
			}
			if (left) {
				mpt_stream_flush(stream);
			}
			/* add data to queue */
			mpt_qpush(&stream->_wd, curr, data);
			data = ((char *) data) + curr;
			continue;
		}
		if (!(flags & MPT_ENUM(StreamWriteBuf))) {
			break;
		}
		/* flush to make room for new data */
		if (file < 0 && (file = _mpt_stream_fwrite(&stream->_info)) < 0) {
			break;
		}
		(void) mpt_queue_save(&stream->_wd, file);
		
		left = stream->_wd.max - stream->_wd.len;
		
		if (left < part && !mpt_queue_prepare(&stream->_wd, part)) {
			break;
		}
	} while (tchunk < count);
	
	return tchunk;
}

