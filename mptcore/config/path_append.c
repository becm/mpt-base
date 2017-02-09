
#include <errno.h>
#include <limits.h>
#include <string.h>

#include "array.h"
#include "config.h"

/*!
 * \ingroup mptConfig
 * \brief reserve data on path
 * 
 * Set character in unused path area.
 * Overwrite while no valid data indicated.
 * 
 * \param path  mpt path data
 * \param len   append data size
 * 
 * \return address of new data
 */
extern void *mpt_path_append(MPT_STRUCT(path) *path, size_t len)
{
	MPT_STRUCT(array) arr;
	size_t pos;
	char *dest = (void *) path->base;
	
	/* need new storage */
	if (!(arr._buf = (void *) dest) || !(path->flags & MPT_PATHFLAG(HasArray))) {
		arr._buf = 0;
		pos = path->off + path->len + path->valid;
		if (!(dest = mpt_array_insert(&arr, 0, pos+len))) {
			return 0;
		}
		/* 'nonnull' false positive: pos!=0 -> path->base!=null */
		path->base = pos ? memcpy(dest, path->base, pos) : dest;
		path->flags |= MPT_PATHFLAG(HasArray);
		return dest + pos;
	}
	/* append to buffer end */
	pos = (--arr._buf)->used;
	
	if (!len) return dest + pos;
	
	/* extend storage */
	if (!(dest = mpt_array_slice(&arr, pos, len)))
		return 0;
	
	path->base = dest - pos;
	
	return dest;
}
