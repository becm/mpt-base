/*!
 * MPT core library
 *   convert type to string pointer/slice
 */

#include <ctype.h>
#include <string.h>

#include <sys/uio.h>

#include "meta.h"
#include "array.h"

#include "convert.h"

/*!
 * \ingroup mptConvert
 * \brief get string data
 * 
 * Convert typed data pointer to string
 * or slice of characters
 * 
 * \param[in,out] from pointer to current string position
 * \param         type convertion target type
 * \param[out]    len  string length
 * 
 * \return length of converted type
 */
extern const char *mpt_data_tostring(const void **from, int type, size_t *len)
{
	static const char def[] = "\0";
	const char *base;
	
	/* simple pointer */
	if (type == 's') {
		const char * const *txt = *from;
		if (!(base = *txt)) {
			base = def;
		}
		if (len) {
			*len = strlen(base);
		}
		*from = txt + 1;
		return base;
	}
	/* data is text array */
	if (type == MPT_ENUM(TypeArray)) {
		const MPT_STRUCT(type_traits) *info;
		const MPT_STRUCT(array) *arr = *from;
		MPT_STRUCT(buffer) *b;
		
		if (!(b = arr->_buf)) {
			*from = arr + 1;
			if (len) *len = 0;
			return def;
		}
		if (!(info = b->_typeinfo)
		    || info->type != 'c') {
			return 0;
		}
		base = (void *) (b + 1);
		if (len) {
			*len = b->_used;
		}
		else if (!b->_used) {
			base = def;
		}
		else if (!memchr(base, 0, b->_used)) {
			return 0;
		}
		*from = arr + 1;
		return (char *) (b + 1);
	}
	/* data is text vector */
	if (type == MPT_value_toVector('c')) {
		const struct iovec *vec = *from;
		
		base = vec->iov_base;
		
		if (!vec || (vec->iov_len && !base)) {
			return 0;
		}
		if (!base) {
			base = def;
		}
		if (len) {
			*len = vec->iov_len;
		}
		else if (!vec->iov_len || base[vec->iov_len - 1]) {
			return 0;
		}
		*from = vec + 1;
		return base;
	}
	return 0;
}
