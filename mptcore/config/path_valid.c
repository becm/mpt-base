
#include <inttypes.h>

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
	ssize_t post;
	
	if (!(buf = (void *) path->base)) {
		return 0;
	}
	if (!(path->flags & MPT_PATHFLAG(HasArray))) {
		return 0;
	}
	--buf;
	post  = buf->_used;
	post -= path->off;
	post -= path->len;
	
	if (post < 0) {
		return MPT_ERROR(BadValue);
	}
	if (post) {
		path->flags |= MPT_PATHFLAG(KeepPost);
	}
	return post;
}
