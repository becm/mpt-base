
#include <string.h>

#include "array.h"
#include "config.h"

/*!
 * \ingroup mptConfig
 * \brief remove path element
 * 
 * remove data and last element from path.
 * 
 * \param path  mpt path descriptor
 * 
 * \return length of removed element
 */
extern int mpt_path_del(MPT_STRUCT(path) *path)
{
	uint8_t *data;
	size_t len, pos, part;
	
	if (!(len = path->len)) {
		return MPT_ERROR(MissingData);
	}
	data = (uint8_t *) path->base;
	pos  = len + path->off;
	
	/* binary linked size separation format */
	if (path->flags & MPT_PATHFLAG(SepBinary)) {
		/* total length error */
		if (len < 2 || len <= (part = data[pos-2])) {
			return MPT_ERROR(BadValue);
		}
		len -= part + 2;
		pos  = len + path->off;
		
		/* forward/backward inconsistency */
		if (part != (pos ? data[pos-1] : path->first)) {
			return MPT_ERROR(BadOperation);
		}
	}
	else {
		/* start on character before assign */
		data += (pos - 2);
		part  = 0;
		
		/* find last separator */
		while (--len && (*(data) != path->sep)) {
			++part; --data;
		}
	}
	if (path->flags & MPT_PATHFLAG(HasArray)) {
		MPT_STRUCT(array) arr;
		arr._buf = (void *) path->base;
		pos = (--arr._buf)->_used;
		if (len > pos) {
			return MPT_ERROR(BadValue);
		}
		pos = len;
		if (!(data = mpt_array_slice(&arr, 0, pos))) {
			return MPT_ERROR(BadOperation);
		}
		path->base = (char *) data;
		arr._buf->_used = pos;
	}
	
	if (!(path->len = len)) {
		path->first = 0;
	}
	path->flags &= ~MPT_PATHFLAG(KeepPost);
	
	return part;
}

/*!
 * \ingroup mptConfig
 * \brief remove post data
 * 
 * invalidate buffer data after path.
 * 
 * \param path  mpt path descriptor
 * 
 * \retval MissingData  no/bad path data
 * \retval 0  no modification needed
 * \retval 1  used size modified
 * \retval 2  created copy with new size
 */
extern int mpt_path_invalidate(MPT_STRUCT(path) *path)
{
	MPT_STRUCT(array) arr;
	size_t used, len = path->off + path->len;
	char *base;
	int flags;
	
	if (!(arr._buf = (void *) path->base)) {
		return len ? -2 : 0;
	}
	if (!(path->flags & MPT_PATHFLAG(HasArray))) {
		return 0;
	}
	used = (--arr._buf)->_used;
	
	if (len > used) {
		return MPT_ERROR(BadValue);
	}
	if (len == used) {
		path->flags &= ~MPT_PATHFLAG(KeepPost);
		return 0;
	}
	/* can modify local copy */
	flags = arr._buf->_vptr->get_flags(arr._buf);
	if (!(MPT_ENUM(BufferShared) & flags)
	 && !(MPT_ENUM(BufferImmutable) & flags)) {
		path->flags &= ~MPT_PATHFLAG(KeepPost);
		arr._buf->_used = len;
		((char *) path->base)[len] = 0;
		return 1;
	}
	/* create local buffer copy */
	if (!(base = mpt_array_slice(&arr, 0, len))) {
		return -1;
	}
	if (arr._buf->_size > len) {
		base[len] = '\0';
	}
	arr._buf->_used = len;
	path->base = base;
	path->flags &= ~MPT_PATHFLAG(KeepPost);
	return 2;
}
