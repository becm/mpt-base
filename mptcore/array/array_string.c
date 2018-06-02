/*!
 * zero termination
 */

#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "array.h"

/*!
 * \ingroup mptArray
 * \brief enshure zero-termination
 * 
 * Check/Set array data zero-terminated.
 * 
 * \param arr  data array
 * 
 * \return start address of string
 */
extern char *mpt_array_string(MPT_STRUCT(array) *arr)
{
	const MPT_STRUCT(type_traits) *info;
	MPT_STRUCT(buffer) *buf;
	char *str, *sep;
	size_t len;
	
	if (!(buf = arr->_buf)) {
		errno = EFAULT;
		return 0;
	}
	/* accept character data only */
	if (!(info = buf->_typeinfo) || info->type != 'c') {
		errno = EINVAL;
		return 0;
	}
	str = (char *) (buf + 1);
	len = buf->_used;
	
	/* string termination in buffer data */
	if ((sep = memchr(str, 0, len))) {
		return str;
	}
	if (!(sep = mpt_array_slice(arr, len, 1))) {
		return 0;
	}
	str = (char *) (buf + 1);
	*sep = '\0';
	
	return str;
}
