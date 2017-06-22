/*!
 * parse value output format
 */

#include <stdlib.h>
#include <string.h>

#include "array.h"

#include "convert.h"

static MPT_INTERFACE_VPTR(buffer) valfmtCtl;
static MPT_STRUCT(buffer) *valfmtCreate(long len)
{
	MPT_STRUCT(buffer) *b;
	if (len < 0) {
		len = 8;
	}
	len = sizeof(*b) + len * sizeof(MPT_STRUCT(valfmt));
	len = MPT_align(len);
	
	if (!(b = malloc(len))) {
		return 0;
	}
	b->_vptr = &valfmtCtl;
	b->_ref._val = 0;
	b->_size = len - sizeof(*b);
	b->_used = 0;
	return b;
}

static void valfmtUnref(MPT_INTERFACE(unrefable) *ref)
{
	MPT_STRUCT(buffer) *b = (void *) ref;
	if (!mpt_refcount_lower(&b->_ref)) {
		free(b);
	}
}
static MPT_STRUCT(buffer) *valfmtDetach(MPT_STRUCT(buffer) *b, long len)
{
	MPT_STRUCT(valfmt) *fmt;
	if (len < 0) {
		len = b->_used / sizeof(*fmt);
	}
	if (b->_ref._val > 1) {
		MPT_STRUCT(buffer) *c;
		long used;
		if (!(c = valfmtCreate(len))) {
			return 0;
		}
		len *= sizeof(*fmt);
		mpt_refcount_lower(&b->_ref);
		used = b->_used;
		if (used > len) {
			used = len;
		}
		c->_used = used;
		memcpy(c + 1, b + 1, used);
		return c;
	}
	len *= sizeof(*fmt);
	if (len <= (long) b->_size) {
		if (len < (long) b->_used) {
			b->_used = len;
		}
		return b;
	}
	len += sizeof(*b);
	len = MPT_align(len);
	if (!(b = realloc(b, len))) {
		return 0;
	}
	b->_size = len - sizeof(*b);
	return b;
}
static int valfmtType(const MPT_STRUCT(buffer) *b)
{
	(void) b;
	return MPT_ENUM(TypeValFmt);
}
static MPT_INTERFACE_VPTR(buffer) valfmtCtl = {
	{ valfmtUnref },
	valfmtDetach,
	valfmtType
};
/*!
 * \ingroup mptConvert
 * \brief parse value format elements
 * 
 * Append value formats in string.
 * 
 * \param arr   value format target array
 * \param base  format descriptions
 * 
 * \return consumed length
 */
extern int mpt_valfmt_add(_MPT_UARRAY_TYPE(valfmt) *arr, MPT_STRUCT(valfmt) fmt)
{
	MPT_STRUCT(buffer) *b;
	MPT_STRUCT(valfmt) *dest;
	long len;
	if ((b = arr->_buf)
	  && b->_vptr->content(b) != valfmtType(0)) {
		b = 0;
	}
	if (!b) {
		if (!(b = valfmtCreate(-1))) {
			return MPT_ERROR(BadOperation);
		}
		memcpy(b + 1, &fmt, sizeof(fmt));
		b->_used = sizeof(fmt);
		arr->_buf = b;
		return 1;
	}
	len = b->_used / sizeof(fmt);
	if (!(b = b->_vptr->detach(b, len + 1))) {
		return 0;
	}
	dest = (void *) (b + 1);
	memcpy(dest + len, &fmt, sizeof(fmt));
	b->_used = (len + 1) * sizeof(fmt);
	return len + 1;
}
