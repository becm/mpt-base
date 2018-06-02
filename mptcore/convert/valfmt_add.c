/*!
 * MPT core library
 *   add value format element
 */

#include <string.h>

#include "array.h"

#include "convert.h"
/*!
 * \ingroup mptConvert
 * \brief add to value format array
 * 
 * Create buffer for and add data to array
 * for value output format elements.
 * 
 * \param arr   value format target array
 * \param base  format descriptions
 * 
 * \return consumed length
 */
extern int mpt_valfmt_add(_MPT_UARRAY_TYPE(valfmt) *arr, MPT_STRUCT(valfmt) fmt)
{
	const MPT_STRUCT(type_traits) *info;
	MPT_STRUCT(buffer) *b;
	MPT_STRUCT(valfmt) *dest;
	long len;
	
	if (!(b = arr->_buf)) {
		static const MPT_STRUCT(type_traits) _valfmt_info = MPT_TYPETRAIT_INIT(MPT_STRUCT(valfmt), MPT_ENUM(TypeValFmt));
		if (!(b = _mpt_buffer_alloc(8 * sizeof(fmt), 0))) {
			return MPT_ERROR(BadOperation);
		}
		memcpy(b + 1, &fmt, sizeof(fmt));
		b->_typeinfo = &_valfmt_info;
		b->_used = sizeof(fmt);
		arr->_buf = b;
		return 1;
	}
	if (!(info = b->_typeinfo)
	    || info->type != MPT_ENUM(TypeValFmt)) {
		return MPT_ERROR(BadType);
	}
	len = b->_used / sizeof(sizeof(fmt));
	if (!(b = b->_vptr->detach(b, (len + 8) * sizeof(fmt)))) {
		return 0;
	}
	arr->_buf = b;
	dest = (void *) (b + 1);
	memcpy(dest + len, &fmt, sizeof(fmt));
	++len;
	b->_used = len * sizeof(fmt);
	return len;
}
