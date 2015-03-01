
#include <errno.h>
#include <string.h>

#include "config.h"

/*!
 * \ingroup mptConfig
 * \brief consume path element
 * 
 * remove first element from path.
 * 
 * \param path  path information
 * 
 * \return removed element length
 */
extern int mpt_path_next(MPT_STRUCT(path) *path)
{
	const char *data;
	size_t len, skip;
	
	if (!(len = path->len)) {
		errno = EINVAL; return -2;
	}
	if (!(data = path->base)) {
		errno = EFAULT; return -1;
	}
	data += path->off;
	
	if (path->flags & MPT_ENUM(PathSepBinary)) {
		skip = (len = path->first) + 2;
		path->first = data[len + 1];
	}
	else if ((skip = path->first)) {
		len = skip++;
		path->first = 0;
	}
	else {
		char	*end = memchr(data, path->sep, len-1);
		skip = end ? (end - data) + 1 : (ssize_t) len;
		len = skip-1;
	}
	path->off += skip;
	path->len -= skip;
	
	return len;
}

