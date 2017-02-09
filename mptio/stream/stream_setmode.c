/*!
 * set stream mode flags and queues.
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/mman.h>

#include "queue.h"
#include "stream.h"

extern int mpt_stream_setmode(MPT_STRUCT(stream) *stream, int mode)
{
	static const MPT_STRUCT(queue) qinit = MPT_QUEUE_INIT;
	int flags;
	
	if (mode & 0xf) {
		errno = EINVAL;
		return -1;
	}
	flags = mpt_stream_flags(&stream->_info);
	
	/* keep read queue */
	if (mode & (MPT_STREAMFLAG(ReadBuf))) {
		mode |= flags & (MPT_STREAMFLAG(ReadBuf) | MPT_STREAMFLAG(ReadMap));
	}
	/* clear read queue */
	else {
		if (stream->_rd._dec) {
			stream->_rd._dec(&stream->_rd._state, 0, 0);
		}
		if (stream->_rd.data.max) {
			if (flags & MPT_STREAMFLAG(ReadMap)) {
				munmap(stream->_rd.data.base, stream->_rd.data.max);
			}
			else if (flags & MPT_STREAMFLAG(ReadBuf)) {
				free(stream->_rd.data.base);
			}
		}
		/* invalidate read queue */
		stream->_rd.data = qinit;
	}
	/* keep write queue */
	if (mode & (MPT_STREAMFLAG(WriteBuf))) {
		mode |= flags & (MPT_STREAMFLAG(WriteBuf) | MPT_STREAMFLAG(WriteMap));
	}
	/* clear write queue */
	else {
		if (stream->_wd._enc) {
			stream->_wd._enc(&stream->_wd._state, 0, 0);
		}
		if (stream->_wd.data.max) {
			if (flags & MPT_STREAMFLAG(WriteMap)) {
				munmap(stream->_wd.data.base, stream->_wd.data.max);
			}
			else if (flags & MPT_STREAMFLAG(WriteBuf)) {
				free(stream->_wd.data.base);
			}
		}
		/* invalidate write queue */
		stream->_wd.data = qinit;
	}
	stream->_info._fd = (stream->_info._fd & ~0xfc) | mode;
	
	return mode;
}
