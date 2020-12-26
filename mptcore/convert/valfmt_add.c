/*!
 * MPT core library
 *   add value format element
 */

#include <string.h>

#include "array.h"
#include "types.h"

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
extern int mpt_valfmt_add(_MPT_UARRAY_TYPE(value_format) *arr, MPT_STRUCT(value_format) fmt)
{
	const MPT_STRUCT(type_traits) *traits = mpt_type_traits(MPT_ENUM(TypeValFmt));
	MPT_STRUCT(buffer) *b;
	MPT_STRUCT(value_format) *dest;
	long len;
	
	if (!traits) {
		return MPT_ERROR(BadOperation);
	}
	
	if (!(b = arr->_buf)) {
		if (!(b = _mpt_buffer_alloc(8 * sizeof(fmt)))) {
			return MPT_ERROR(BadOperation);
		}
		memcpy(b + 1, &fmt, sizeof(fmt));
		b->_used = sizeof(fmt);
		b->_content_traits = traits;
		arr->_buf = b;
		return 1;
	}
	if (traits != b->_content_traits) {
		return MPT_ERROR(BadType);
	}
	len = b->_used / sizeof(fmt);
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
