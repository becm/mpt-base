
#include <errno.h>
#include <limits.h>
#include <string.h>

#include "array.h"
#include "config.h"

/*!
 * \ingroup mptConfig
 * \brief finalize path
 * 
 * Clear (extended) path data
 * 
 * \param path  path information
 */
extern void mpt_path_fini(MPT_STRUCT(path) *path)
{
	MPT_STRUCT(buffer) *buf;
	
	if (!(path->flags & MPT_PATHFLAG(HasArray))) {
		return;
	}
	path->flags &= ~MPT_PATHFLAG(HasArray);
	path->off = path->len = path->valid = 0;
	
	if (!(buf = (void *) path->base)) {
		return;
	}
	--buf;
	
	if (buf->shared) {
		--buf->shared;
	}
	else if (buf->resize) {
		buf->resize(buf, 0);
	}
	path->base = 0;
}
