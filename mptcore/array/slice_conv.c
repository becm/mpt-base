/*!
 * configure and prepare bound solver.
 */

#include <string.h>

#include <sys/uio.h>

#include "array.h"

#include "meta.h"

#include "convert.h"

/*!
 * \ingroup mptArray
 * \brief get values from slice
 * 
 * Convert zero-deliminated data on slice to text elements.
 * 
 * \param s     data slice descriptor
 * \param type  target type code
 * \param data  value address
 * 
 * \return source type and applied flags
 */
extern int mpt_slice_conv(const MPT_STRUCT(slice) *s, int type, void *data)
{
	const char *base, *end;
	int len;
	
	if (!type) {
		static const char types[] = { MPT_value_toVector('c'), 's', 0 };
		if (data) *((const char **) data) = types;
		return 0;
	}
	if (!(len = s->_len)) {
		return 0;
	}
	base = ((char *) (s->_a._buf + 1)) + s->_off;
	
	if (type == MPT_value_toVector('c')
	    || type == MPT_ENUM(TypeVector)) {
		struct iovec *vec;
		if ((vec = data)) {
			vec->iov_base = (char *) base;
			vec->iov_len  = len;
		}
		return s->_off ? type : MPT_ENUM(TypeBuffer);
	}
	if (!(end = memchr(base, 0, len))) {
		return MPT_ERROR(BadValue);
	}
	if (type == MPT_ENUM(TypeValue)) {
		MPT_STRUCT(value) *val;
		
		if ((val = data)) {
			if (!end) {
				return MPT_ERROR(BadValue);
			}
			val->fmt = 0;
			val->ptr = base;
		}
		return 's';
	}
	if (type == 's') {
		if (data) {
			if (!end) {
				return MPT_ERROR(BadValue);
			}
			*((const char **) data) = base;
		}
		return MPT_value_toVector('c');
	}
	if (type == 'k') {
		if (data && !(base = mpt_convert_key(&base, 0, 0))) {
			return MPT_ERROR(BadType);
		}
		return MPT_value_toVector('c');
	}
	if ((len = mpt_convert_string(base, type, data)) <= 0) {
		return len;
	}
	return MPT_value_toVector('c');
}
