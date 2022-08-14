/*!
 * set text string pointer data.
 */

#include <string.h>
#include <stdlib.h>

#include <sys/uio.h>

#include "types.h"

#include "layout.h"

/*!
 * \ingroup mptLayout
 * \brief change text
 * 
 * Set character data.
 * 
 * \param ptr  string pointer reference
 * \param data data source to change string
 * \param len  new data length
 * 
 * \return new data length
 */
extern int mpt_string_set(char **ptr, const char *data, int len)
{
	char *txt;
	
	if (!data) {
		len = 0;
	}
	else if (len < 0) {
		len = strlen(data);
	}
	if (!len) {
		free(*ptr);
		*ptr = 0;
		return 0;
	}
	if (data == *ptr) {
		return 0;
	}
	if (!(txt = realloc(*ptr, len + 1))) {
		return MPT_ERROR(BadOperation);
	}
	*ptr = memcpy(txt, data, len);
	txt[len] = 0;
	
	return len;
}

/*!
 * \ingroup mptLayout
 * \brief change text
 * 
 * Get/Set character data.
 * 
 * \param ptr  string pointer reference
 * \param src  data source to change string
 * 
 * \return consumed/changed value
 */
extern int mpt_string_pset(char **ptr, MPT_INTERFACE(convertable) *src)
{
	struct iovec vec;
	int len;
	
	if (!src) {
		return (ptr && *ptr) ? strlen(*ptr) : 0;
	}
	if ((len = src->_vptr->convert(src, MPT_type_toVector('c'), &vec)) > 0) {
		if ((len = mpt_string_set(ptr, vec.iov_base, vec.iov_len)) < 0) {
			return len;
		}
		return 0;
	}
	vec.iov_base = 0;
	if ((len = src->_vptr->convert(src, 's', &vec.iov_base)) < 0) {
		return len;
	}
	if (!len || !vec.iov_base) {
		mpt_string_set(ptr, 0, 0);
	}
	else if ((len = mpt_string_set(ptr, vec.iov_base, -1) < 0)) {
		return len;
	}
	return 0;
}
