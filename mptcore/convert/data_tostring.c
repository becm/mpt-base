/*!
 * MPT core library
 *   convert type to string pointer/slice
 */

#include <ctype.h>
#include <string.h>
#include <errno.h>

#include <sys/uio.h>

#include "array.h"
#include "types.h"

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
	
	/* simple string pointer */
	if (type == 's') {
		const char * const *txt = *from;
		if (!txt || !(base = *txt)) {
			base = def;
		}
		if (len) {
			*len = strlen(base);
		}
		if (txt) {
			*from = txt + 1;
		}
		return base;
	}
	/* text buffer (pointer or tracked reference) */
	if (type == MPT_ENUM(TypeArray)
	 || type == MPT_ENUM(TypeBufferPtr)) {
		static const MPT_STRUCT(type_traits) *traits= 0;
		const MPT_STRUCT(array) *arr;
		const MPT_STRUCT(buffer) *b;
		
		if (!(arr = *from)) {
			errno = EINVAL;
			return 0;
		}
		if (!(b = arr->_buf)) {
			errno = ENOTSUP;
			return 0;
		}
		/* initialize traits binding */
		if (!traits && (!(traits = mpt_type_traits('c')))) {
			errno = ENOTSUP;
			return 0;
		}
		if (b->_content_traits != traits) {
			errno = EINVAL;
			return 0;
		}
		base = (const char *) (b + 1);
		if (len) {
			*len = b->_used;
		}
		else if (!b->_used) {
			base = def;
		}
		else if (!memchr(base, 0, b->_used)) {
			errno = EINVAL;
			return 0;
		}
		*from = arr + 1;
		return base;
	}
	/* vector data with text content */
	if (type == MPT_type_toVector('c')) {
		const struct iovec *vec = *from;
		
		if (!vec) {
			errno = EINVAL;
			return 0;
		}
		if (!(base = vec->iov_base)) {
			if (vec->iov_len) {
				errno = EINVAL;
				return 0;
			}
			base = def;
		}
		if (len) {
			*len = vec->iov_len;
		}
		else if (!vec->iov_len || base[vec->iov_len - 1]) {
			errno = EINVAL;
			return 0;
		}
		*from = vec + 1;
		return base;
	}
	errno = EINVAL;
	return 0;
}
