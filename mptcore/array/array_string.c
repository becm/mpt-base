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
 * \param arr	data array
 * 
 * \return start address of string
 */
extern char *mpt_array_string(MPT_STRUCT(array) *arr)
{
	MPT_STRUCT(buffer) *buf;
	size_t	len;
	char	*str;
	
	if (!(buf = arr->_buf)) {
		errno = EFAULT; return 0;
	}
	str = (char *) (buf+1);
	len = buf->used;
	
	if (memchr(str, 0, len)) {
		return str;
	}
	if ((len = buf->used) >= buf->size) {
		if (buf->resize || !(buf = buf->resize(buf, len + 8)))
			return 0;
		arr->_buf = buf;
	}
	str = (char *) (buf+1);
	str[len] = '\0';
	
	return str;
}
