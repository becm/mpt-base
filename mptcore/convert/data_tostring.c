
#include <ctype.h>
#include <string.h>

#include <sys/uio.h>

#include "array.h"

#include "convert.h"

/*!
 * \ingroup mptConvert
 * \brief get string data
 * 
 * convert typed data pointer to string
 * 
 * \param[in,out] from pointer to current string position
 * \param         type convertion target type
 * \param[out]    len  string length
 * 
 * \return length of converted type
 */
extern const char *mpt_data_tostring(const void **from, int type, size_t *len)
{
	static const char def[] = "";
	const char *base;
	size_t used;
	
	if (type == 's') {
		if (!(base = *from)) {
			base = def;
		}
		if (len) {
			*len = strlen(base);
		}
		*from = ((uint8_t *) from) + sizeof(char *);
		return base;
	}
	if (type == 'C') {
		const MPT_STRUCT(array) *a = *from;
		size_t size;
		
		if (!a) {
			return 0;
		}
		if (a->_buf) {
			base = (char *) (a->_buf+1);
			used = a->_buf->used;
			size = a->_buf->size;
		} else {
			base = def;
			used = 0;
			size = 1;
		}
		if (len) {
			*len = used;
		}
		else if (used >= size || base[used]) {
			return 0;
		}
		*from = ((uint8_t *) from) + sizeof(*a);
		return base;
	}
	if (type == ('c' | MPT_ENUM(TypeVector))) {
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
		*from = ((uint8_t *) from) + sizeof(*vec);
		return base;
	}
	return 0;
}
