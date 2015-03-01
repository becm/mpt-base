
#include <errno.h>
#include <limits.h>

#include "array.h"
#include "config.h"

/*!
 * \ingroup mptConfig
 * \brief set path data valid
 * 
 * append remaining array data to path valid data size.
 * 
 * \param path path information
 * 
 * \return type of operation
 */
extern int mpt_path_valid(MPT_STRUCT(path) *path)
{
	MPT_STRUCT(buffer) *buf;
	size_t post;
	
	if (!(buf = (void *) path->base)) {
		return -1;
	}
	if (!(path->flags & MPT_ENUM(PathHasArray))) {
		return 0;
	}
	post  = buf[-1].used;
	post -= path->off;
	post -= path->len;
	
	if (post > UINT16_MAX) {
		errno = EOVERFLOW;
		return -2;
	}
	return path->valid = post;
}

