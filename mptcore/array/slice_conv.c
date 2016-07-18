/*!
 * configure and prepare bound solver.
 */

#include <string.h>

#include <sys/uio.h>

#include "convert.h"

/*!
 * \ingroup mptArray
 * \brief get strings from slice
 * 
 * Convert zero-deliminated data on slice to text elements.
 * 
 * \param src  data slice descriptor
 * \param sep  argument separator
 * \param msg  message data
 * 
 * \return hint to event controller (int)
 */
extern int mpt_slice_conv(MPT_INTERFACE(slice) *s, int type, void *data)
{
	char *base, *end;
	size_t len;
	
	if (!(len = s->_len)) {
		return 0;
	}
	base = ((char *) (s->_a._buf + 1)) + s->_off;
	
	if ((end = memchr(base, 0,len))) {
		len = end - base;
	}
	if (!(type & 0xff)) {
		if (data) {
			return MPT_ERROR(BadOperation);
		}
		return len;
	}
	if((type & 0xff) == MPT_ENUM(TypeValue)) {
		MPT_STRUCT(value) *val;
		
		if ((val = data)) {
			if (!end) {
				return MPT_ERROR(BadValue);
			}
			val->fmt = 0;
			val->ptr = base;
		}
		type = (type & MPT_ENUM(ValueConsume)) | MPT_ENUM(TypeValue);
	}
	if ((type & 0xff) == MPT_ENUM(TypeProperty)) {
		char *sep = 0;
		int assign = 0;
		
		if (data && !end) {
			return MPT_ERROR(BadValue);
		}
		if (type & MPT_ENUM(AssignEqual)) {
			sep = memchr(base, '=', len);
			assign = MPT_ENUM(AssignEqual);
		}
		if (!sep && type & MPT_ENUM(AssignColon)) {
			sep = memchr(base, ':', len);
			assign = MPT_ENUM(AssignColon);
		}
		if (sep) {
			/* needs data change */
			if (!(type & MPT_ENUM(ValueConsume))) {
				return MPT_ERROR(BadValue);
			}
		} else {
			sep = base - 1;
			base = 0;
			type = (type & MPT_ENUM(ValueConsume)) | MPT_ENUM(TypeValue);
		}
		if (data) {
			MPT_STRUCT(property) *pr = data;
			
			pr->name = base;
			pr->desc = base ? sep : 0;
			pr->val.fmt = 0;
			pr->val.ptr = sep + 1;
		}
		type = (type &  MPT_ENUM(ValueConsume)) | assign | MPT_ENUM(TypeProperty);
	}
	else if ((type & 0xff) == MPT_value_toVector('c')) {
		struct iovec *vec;
		if ((vec = data)) {
			vec->iov_base = base;
			vec->iov_len  = len;
		}
		type = (type & MPT_ENUM(ValueConsume)) | MPT_value_toVector('c');
	}
	else if ((type & 0xff) == 's') {
		if (data) {
			if (!end) {
				return MPT_ERROR(BadValue);
			}
			*((const char **) data) = base;
		}
		type = (type & MPT_ENUM(ValueConsume)) | 's';
	}
	else {
		int curr;
		if (!end) {
			return MPT_ERROR(BadValue);
		}
		if ((curr = mpt_convert_string(base, type, data)) < 0) {
			return curr;
		}
		if (!curr || curr < (int) len) {
			len = curr;
			end = 0;
		}
	}
	
	if (type & MPT_ENUM(ValueConsume)) {
		if (end) ++len;
		s->_off += len;
		s->_len -= len;
	}
	return type;
}
