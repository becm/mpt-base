
#include <string.h>
#include <errno.h>

#include "array.h"
#include "config.h"

/*!
 * \ingroup mptConfig
 * \brief get last path element
 * 
 * change path to contain only last element.
 * 
 * \param path  path information
 * 
 * \return length of last element
 */
extern int mpt_path_last(MPT_STRUCT(path) *path)
{
	const char *data;
	size_t len, pos;
	
	if (!(pos = path->len) || !(data = path->base)) {
		errno = EINVAL; return -2;
	}
	if (path->flags & MPT_PATHFLAG(SepBinary)) {
		if (pos < 2 || pos < (len = data[pos-2])) {
			errno = EINVAL; return -2;
		}
		pos -= len;
		
		path->off += pos;
		path->len  = (path->first = len) + 2;
		
		return len;
	}
	
	data += (--pos - 1);
	len = 0;
	
	/* find last separator */
	while (pos && *data != path->sep) {
		--data; ++len; --pos;
	}
	path->off += pos;
	path->len  = (path->first = len) + 1;
	
	return len;
}
