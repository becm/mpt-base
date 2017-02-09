
#include <errno.h>
#include <limits.h>
#include <string.h>

#include "array.h"
#include "config.h"

/*!
 * \ingroup mptConfig
 * \brief add path element
 *
 * add valid size to path.
 * 
 * \param path  path data
 * 
 * \return operation code
 */
extern int mpt_path_add(MPT_STRUCT(path) *path)
{
	MPT_STRUCT(array) arr;
	size_t post, len, pre;
	char *data;
	
	if (!(data = (char *) path->base)) return -1;
	len = path->off + path->len;
	
	if (path->flags & MPT_PATHFLAG(HasArray)) {
		arr._buf = (void *) data;
		--arr._buf;
		pre  = 0;
		post = arr._buf->used - len - path->valid;
	} else {
		arr._buf = 0;
		pre = len + path->valid;
		post = 0;
	}
	/* binary linked size separation format */
	if (path->flags & MPT_PATHFLAG(SepBinary)) {
		if (path->valid > UINT8_MAX) {
			errno = ERANGE; return -2;
		}
		if (post < 2U) {
			if (!mpt_array_append(&arr, pre+2-post, 0))
				return -1;
			data = pre ? memcpy(arr._buf+1, data, pre) : (void*) (arr._buf+1);
		}
		/* set leading/trailing/next size parameter */
		if (len) data[len - 1] = path->valid;
		else path->first = path->valid;
		
		/* set next part */
		len += path->valid;
		data[len++] = path->valid;
		data[len++] = 0;
	}
	else {
		/* separator must not be in part */
		if (memchr(data + len, path->sep, path->valid)) {
			errno = EINVAL; return -3;
		}
		if (post < 1U) {
			if (!mpt_array_append(&arr, pre+1, 0))
				return -1;
			data = pre ? memcpy(arr._buf+1, data, pre) : (void*) (arr._buf+1);
		}
		/* change path assign to separator */
		if (len) data[len - 1] = path->sep;
		else path->first = path->valid;
		
		/* set next part */
		len += path->valid;
		data[len++] = path->assign;
	}
	path->base  = data;
	path->len   = len - path->off;
	path->valid = 0;
	
	return 0;
}
