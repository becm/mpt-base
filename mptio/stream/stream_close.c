/*!
 * close stream descriptor
 */

#include <stdlib.h>

#include "queue.h"
#include "stream.h"

extern int mpt_stream_close(MPT_STRUCT(stream) *stream)
{
	int flags = mpt_stream_flags(&stream->_info);
	mpt_stream_flush(stream);
	_mpt_stream_setfile(&stream->_info, -1, -1);
	mpt_stream_setmode(stream, 0);
	return (flags & MPT_STREAMFLAG(MesgActive)) ? 1 : 0;
}

