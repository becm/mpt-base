
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
extern int mpt_path_add(MPT_STRUCT(path) *path, int add)
{
	MPT_STRUCT(array) arr;
	size_t post, len, pre;
	char *data;
	
	if (!(data = (char *) path->base)) {
		return MPT_ERROR(MissingBuffer);
	}
	len = path->off + path->len;
	
	if (path->flags & MPT_PATHFLAG(HasArray)) {
		arr._buf = (void *) data;
		--arr._buf;
		pre  = 0;
		post = arr._buf->_used - len;
		if (post < (size_t) add) {
			return MPT_ERROR(BadValue);
		}
		post -= add;
	} else {
		arr._buf = 0;
		pre = len + add;
		post = 0;
	}
	/* binary linked size separation format */
	if (path->flags & MPT_PATHFLAG(SepBinary)) {
		if (add > UINT8_MAX) {
			return MPT_ERROR(BadValue);
		}
		if (post < 2U) {
			if (!mpt_array_append(&arr, pre + 2 - post, 0)) {
				return MPT_ERROR(BadOperation);
			}
			data = pre ? memcpy(arr._buf + 1, data, pre) : (void*) (arr._buf + 1);
		}
		/* set leading/trailing/next size parameter */
		if (len) {
			data[len - 1] = add;
		} else {
			path->first = add;
		}
		/* set next part */
		len += add;
		data[len++] = add;
		data[len++] = 0;
	}
	else {
		/* separator must not be in part */
		if (memchr(data + len, path->sep, add)) {
			return MPT_ERROR(BadValue);
		}
		if (post < 1U) {
			if (!mpt_array_append(&arr, pre + 1, 0)) {
				return MPT_ERROR(BadOperation);
			}
			data = pre ? memcpy(arr._buf + 1, data, pre) : (void*) (arr._buf + 1);
		}
		/* change path assign to separator */
		if (len) {
			data[len - 1] = path->sep;
		} else {
			path->first = add;
		}
		/* set next part */
		len += add;
		data[len++] = path->assign;
	}
	path->base  = data;
	path->len   = len - path->off;
	
	/* remove persistence flag */
	path->flags &= ~MPT_PATHFLAG(KeepPost);
	
	return 0;
}
