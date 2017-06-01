
#include <errno.h>
#include <limits.h>
#include <string.h>

#include "array.h"
#include "config.h"

/*!
 * \ingroup mptConfig
 * \brief add character to path
 * 
 * Set character in unused path area.
 * Overwrite while no valid data indicated.
 * 
 * \param path  MPT path data
 * \param val   character to add
 * 
 * \return type of add operation
 */
extern int mpt_path_addchar(MPT_STRUCT(path) *path, int val)
{
	MPT_STRUCT(array) arr;
	uint8_t *dest;
	size_t off, pos;
	
	pos = path->off + path->len;
	
	/* need new storage */
	if (!(arr._buf = (void *) path->base) || !(path->flags & MPT_PATHFLAG(HasArray))) {
		arr._buf = 0;
		if (!(dest = mpt_array_insert(&arr, 0, pos+1))) {
			return -1;
		}
		/* 'nonnull' false positive: pos!=0 -> path->base!=null */
		path->base = pos ? memcpy(dest, path->base, pos) : dest;
		dest[pos]  = val & 0xff;
		path->flags |= MPT_PATHFLAG(HasArray);
		return 3;
	}
	/* next character position */
	off = (--arr._buf)->used;
	
	/* modify local copy */
	if (!arr._buf->shared) {
		dest = (uint8_t *) (arr._buf+1);
		/* can reuse last existing value */
		if ((pos < off) && !(path->flags & MPT_PATHFLAG(KeepPost))) {
			dest[off-1] = val & 0xff;
			return 0;
		}
		/* append in unused space */
		if (off < arr._buf->size) {
			dest[off++] = val & 0xff;
			arr._buf->used = off;
			return 1;
		}
	}
	/* extend storage */
	if (!(dest = mpt_array_slice(&arr, off, 1))) {
		return -1;
	}
	*dest = val & 0xff;
	
	return 2;
}

/*!
 * \ingroup mptConfig
 * \brief remove character from path
 * 
 * Clear last character path.
 * Reduce valid data size if applicable.
 * 
 * \param path  MPT path data
 * 
 * \return removed character
 */
extern int mpt_path_delchar(MPT_STRUCT(path) *path)
{
	MPT_STRUCT(buffer) *buf;
	size_t len, used;
	
	/* need new storage */
	if (!(buf = (void *) path->base)) {
		return MPT_ERROR(MissingData);
	}
	/* remove post data */
	if (!(path->flags & MPT_PATHFLAG(HasArray))) {
		return MPT_ERROR(MissingBuffer);
	}
	len = path->off + path->len;
	--buf;
	
	/* intersects with used data */
	if ((used = buf->used) <= len) {
		return MPT_ERROR(MissingData);
	}
	buf->used = --used;
	
	return path->base[used];
}
