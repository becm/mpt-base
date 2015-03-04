
#include <errno.h>

#include "stream.h"

/*!
 * \ingroup mptStream
 * \brief get stream character
 * 
 * Read single (unsigned 8bit) character from MPT stream.
 * 
 * \param stream stream to read from
 * 
 * \return next character on stream
 */
extern int mpt_stream_getc(MPT_STRUCT(stream) *stream)
{
	unsigned char curr;
	
	if (!mpt_stream_read(stream, 1, &curr, 1)) {
		return -1;
	}
	return curr;
}

