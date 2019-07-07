
#include <string.h>

#include <sys/un.h>
#include <netinet/in.h>

#include "queue.h"
#include "meta.h"

#include "stream.h"

/*!
 * \ingroup mptStream
 * \brief set stream data
 * 
 * Set stream from source data.
 * 
 * \param stream stream descriptor
 * \param src    data source
 * 
 * \retval >0 success
 * \retval 0  default value
 * \retval <0 error
 */
extern int mpt_stream_setter(MPT_STRUCT(stream) *stream, MPT_INTERFACE(convertable) *src)
{
	const char *path, *arg = 0;
	int len, how;
	
	if (!src) {
		if (_mpt_stream_fread(&stream->_info) >= 0) {
			return (_mpt_stream_fwrite(&stream->_info) >= 0) ? 3 : 1;
		}
		return (_mpt_stream_fwrite(&stream->_info) >= 0) ? 2 : 0;
	}
	
	if ((len = src->_vptr->convert(src, 's', &path)) < 0) {
		return -1;
	}
	
	if (path && (arg = strchr(path, ':'))) {
		const char *tmp = arg+1;
		arg = path;
		path = tmp;
	}
	
	if ((how = mpt_stream_open(stream, path, arg)) < 0) {
		return -2;
	}
	
	return len;
}
