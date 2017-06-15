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
	MPT_STRUCT(buffer) *buf;
	size_t len;
	char *str, *sep;
	int type;
	
	if (!(buf = arr->_buf)) {
		errno = EFAULT;
		return 0;
	}
	/* accept raw and character data only */
	if ((type = buf->_vptr->content(buf))
	  && type != 'c') {
		errno = EINVAL;
		return 0;
	}
	str = (char *) (buf + 1);
	len = buf->_used;
	
	if (!(sep = memchr(str, 0, len))) {
		if (len < buf->_size) {
		    str[len] = '\0';
		    return str;
		}
	}
	if (!(buf = buf->_vptr->detach(buf, len + 1))) {
		return 0;
	}
	arr->_buf = buf;
	
	str = (char *) (buf + 1);
	str[len] = '\0';
	
	/* replace inline separators with space */
	while (sep) {
		size_t pos;
		*sep++ = ' ';
		pos = sep - str;
		sep = memchr(sep, 0, len - pos);
	}
	return str;
}
