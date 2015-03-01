
#include <string.h>
#include <errno.h>

#include "array.h"
#include "config.h"

/*!
 * \ingroup mptConfig
 * \brief get path data
 * 
 * return path data.
 * 
 * \param path  path information
 * 
 * \return path element data
 */
extern const char *mpt_path_data(const MPT_STRUCT(path) *path)
{
	const char *data;
	size_t	len;
	
	if (!path->valid || !(data = path->base)) {
		return 0;
	}
	len  = path->off + path->len;
	
	if (path->flags & MPT_ENUM(PathHasArray)) {
		MPT_STRUCT(buffer) *buf = (void *) data;
		size_t	end = len + path->valid;
		if (buf[-1].used > end) ((char *) data)[end] = 0;
	}
	return data + len;
}

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
	size_t	len, pos;
	
	if (!(pos = path->len) || !(data = path->base)) {
		errno = EINVAL; return -2;
	}
	if (path->flags & MPT_ENUM(PathSepBinary)) {
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
