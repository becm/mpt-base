
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
	size_t len;
	
	if (!(data = path->base)) {
		return 0;
	}
	len  = path->off + path->len;
	
	if (path->flags & MPT_PATHFLAG(HasArray)) {
		MPT_STRUCT(buffer) *buf = (void *) data;
		size_t end = buf[-1].used;
		if (end < buf[-1].size) ((char *) data)[end] = 0;
	}
	return data + len;
}
