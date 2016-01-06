/*!
 * configure and prepare bound solver.
 */

#include <string.h>

#include <sys/uio.h>

#include "array.h"

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
		return MPT_ERROR(MissingData);
	}
	base = ((char *) (s->_a._buf)) + s->_off;
	
	if (!(end = memchr(base, 0,len))) {
		return MPT_ERROR(BadValue);
	}
	len = end - base;
	
	if (!(type & 0xff) || (type & 0xff) == MPT_ENUM(TypeValue)) {
		MPT_STRUCT(value) *val;
		
		if ((val = data)) {
			val->fmt = 0;
			val->ptr = base;
		}
		type = (type & ~0xff) | MPT_ENUM(TypeValue);
	}
	if ((type & 0xff) == MPT_ENUM(TypeProperty)) {
		char *sep;
		int psep;
		
		psep = (type & 0xff0000) / 0x10000;
		
		if (!(sep = memchr(base, psep ? psep : '=', len))) {
			return MPT_ERROR(BadValue);
		}
		/* needs data change */
		if (!(type & MPT_ENUM(ValueConsume))) {
			return MPT_ERROR(BadValue);
		}
		*sep = 0;
		if (data) {
			MPT_STRUCT(property) *pr = data;
			
			pr->name = base;
			pr->desc = 0;
			pr->val.fmt = 0;
			pr->val.ptr = sep + 1;
		}
	}
	else if ((type & 0xff) == ('c' - 0x40)) {
		struct iovec *vec;
		if ((vec = data)) {
			vec->iov_base = base;
			vec->iov_len  = len;
		}
	}
	else if ((type & 0xff) == 's') {
		if (data) {
			*((const char **) data) = base;
		}
	}
	else {
		return MPT_ERROR(BadType);
	}
	
	if (type & MPT_ENUM(ValueConsume)) {
		++len;
		s->_off += len;
		s->_len -= len;
	}
	return type & (MPT_ENUM(ValueConsume) | 0xff);
}
