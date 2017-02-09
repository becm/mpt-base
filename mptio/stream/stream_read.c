/*!
 * read data from stream (and backing file).
 */

#include <errno.h>

#include <unistd.h>

#include "queue.h"
#include "stream.h"

extern size_t mpt_stream_read(MPT_STRUCT(stream) *stream, size_t count, void *data, size_t part)
{
	MPT_STRUCT(queue) *queue = &stream->_rd.data;
	size_t tchunk = 0;
	int flags, file = -1;
	
	if (stream->_rd._dec) {
		errno = EINVAL;
		return 0;
	}
	flags = mpt_stream_flags(&stream->_info);
	
	if (!count) {
		return queue->len;
	}
	/* peek available data */
	if (!part) {
		if ((part = queue->len) < count) {
			if (!(flags & MPT_STREAMFLAG(ReadBuf))) {
				return 0;
			}
			if ((file = _mpt_stream_fread(&stream->_info)) < 0) {
				return 0;
			}
			if (flags & MPT_STREAMFLAG(ReadMap)) {
				return 0;
			}
			if (!mpt_queue_prepare(queue, count - part)) {
				return 0;
			}
			if (mpt_queue_load(queue, file, 0) <= 0) {
				mpt_stream_setmode(stream, flags | MPT_ENUM(ErrorEmpty));
			}
			part = queue->len;
		}
		part /= count;
		
		if (data && part) {
			mpt_queue_get(queue, 0, count, data);
		}
		return part;
	}
	do {
		size_t curr;
		
		/* get available chunks from buffer */
		if ((curr = queue->len / part)) {
			size_t len;
			
			if (curr > (count - tchunk)) {
				len = part * (curr = count - tchunk);
			} else {
				len = part * curr;
			}
			if (!data) {
				if (!mpt_queue_crop(queue, 0, len)) {
				    break;
				}
			}
			else if (!mpt_qshift(queue, len, data)) {
				break;
			}
			tchunk += curr;
			
			data = ((char *) data) + curr;
			continue;
		}
		if (flags & MPT_STREAMFLAG(ReadMap)) {
			break;
		}
		if (file < 0 && (file = _mpt_stream_fread(&stream->_info)) < 0) {
			break;
		}
		if ((curr = queue->max - queue->len) < part) {
			if (!mpt_queue_prepare(queue, part)) {
				break;
			}
		}
		/* load remaining needed data */
		if (mpt_queue_load(queue, file, 0) <= 0) {
			mpt_stream_seterror(&stream->_info, MPT_ENUM(ErrorEmpty)); break;
		}
	} while (tchunk < count);
	
	return tchunk;
}

