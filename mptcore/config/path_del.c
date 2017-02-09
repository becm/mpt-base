
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
		pos = (--arr._buf)->used;
		if (len > pos) {
			return MPT_ERROR(BadValue);
		}
		pos = len + path->off;
		if (arr._buf->shared) {
			if (!(data = mpt_array_slice(&arr, 0, pos))) {
				return -1;
			}
			path->base = (char *) data;
		}
		arr._buf->used = pos;
	}
	
	if (!(path->len = len)) {
		path->first = 0;
	}
	path->valid = 0;
	
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
 * \retval -2 no/bad path data
 * \retval 0  no modification needed
 * \retval 1  used size modified
 * \retval 2  created copy with new size
 */
extern int mpt_path_invalidate(MPT_STRUCT(path) *path)
{
	MPT_STRUCT(array) arr;
	size_t used, len = path->off + path->len;
	
	if (!(arr._buf = (void *) path->base)) {
		return len ? -2 : 0;
	}
	if (!(path->flags & MPT_PATHFLAG(HasArray))) {
		return 0;
	}
	used = (--arr._buf)->used;
	
	if (len > used) {
		return -2;
	}
	if (len == used) {
		return 0;
	}
	if (!arr._buf->shared) {
		path->valid = 0;
		arr._buf->used = len;
		((char *) path->base)[len] = 0;
		return 1;
	}
	/* create local buffer copy */
	if (!(path->base = mpt_array_slice(&arr, 0, len))) {
		return -1;
	}
	path->valid = 0;
	return 2;
}
