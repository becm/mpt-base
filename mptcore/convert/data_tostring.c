
#include <ctype.h>
#include <string.h>

#include <sys/uio.h>

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
	
	/* simple pointer */
	if ((type & 0xff) == 's') {
		const char * const *txt = *from;
		if (!(base = *txt)) {
			base = def;
		}
		if (len) {
			*len = strlen(base);
		}
		if (type & MPT_ENUM(ValueConsume)) {
			*from = txt + 1;
		}
		return base;
	}
	/* data is text array */
	if ((type & 0xff) == MPT_value_toArray('c')) {
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
		if (type & MPT_ENUM(ValueConsume)) {
			*from = a + 1;
		}
		return base;
	}
	/* data is text vector */
	if ((type & 0xff) == MPT_value_toVector('c')) {
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
		if (type & MPT_ENUM(ValueConsume)) {
			*from = vec + 1;
		}
		return base;
	}
	return 0;
}
