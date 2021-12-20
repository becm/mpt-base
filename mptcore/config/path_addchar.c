
#include <errno.h>
#include <limits.h>
#include <string.h>

#include "types.h"

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
	MPT_STRUCT(buffer) *b;
	uint8_t *dest;
	size_t off, pos;
	int flags;
	
	pos = path->off + path->len;
	
	/* need new storage */
	if (!(b = (void *) path->base) || !(path->flags & MPT_PATHFLAG(HasArray))) {
		if (!(b = _mpt_buffer_alloc(pos + 1, 0))) {
			return MPT_ERROR(BadOperation);
		}
		if (!(dest = mpt_buffer_insert(b, 0, pos + 1))) {
			b->_vptr->unref(b);
			return MPT_ERROR(MissingBuffer);
		}
		if (path->base) {
			memcpy(dest, path->base, pos);
		}
		path->base = (void *) (b + 1);
		dest[pos]  = val & 0xff;
		path->flags |= MPT_PATHFLAG(HasArray);
		return 3;
	}
	/* next character position */
	off = (--b)->_used;
	
	/* modify local copy */
	flags = b->_vptr->get_flags(b);
	if (!(MPT_ENUM(BufferShared) & flags)
	 && !(MPT_ENUM(BufferImmutable) & flags)) {
		dest = (uint8_t *) (b + 1);
		/* can reuse last existing value */
		if ((pos < off) && !(path->flags & MPT_PATHFLAG(KeepPost))) {
			dest[off - 1] = val & 0xff;
			return 0;
		}
		/* append in unused space */
		if (off < b->_size) {
			dest[off++] = val & 0xff;
			b->_used = off;
			return 1;
		}
	}
	/* extend storage */
	arr._buf = b;
	if (!(dest = mpt_array_slice(&arr, off, 1))) {
		return -1;
	}
	*dest = val & 0xff;
	path->base = (void *) (arr._buf + 1);
	
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
	int flags;
	
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
	if ((used = buf->_used) <= len) {
		return MPT_ERROR(MissingData);
	}
	/* check permission on buffer */
	flags = buf->_vptr->get_flags(buf);
	if (MPT_ENUM(BufferShared) & flags
	 || MPT_ENUM(BufferImmutable) & flags) {
		/* make private copy */
		if (!(buf = buf->_vptr->detach(buf, used))) {
			return 0;
		}
		path->base = (void *) (buf + 1);
	}
	/* remove character from buffer */
	buf->_used = --used;
	
	return path->base[used];
}
