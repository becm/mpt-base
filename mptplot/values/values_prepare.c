/*!
 * generate values.
 */

#include <string.h>
#include <errno.h>

#include "values.h"

/*!
 * \ingroup mptValues
 * \brief reserve data on array
 * 
 * Prepare new floating point elements on array.
 * Copy old data if length arument is less than zero.
 * 
 * \param arr  array to append data
 * \param len  size to append
 * 
 * \return zeroed/copied start address
 */
extern double *mpt_values_prepare(_MPT_ARRAY_TYPE(double) *arr, long len)
{
	const MPT_STRUCT(type_traits) *info;
	MPT_STRUCT(buffer) *buf;
	uint8_t *data;
	size_t add;
	long used;
	
	if (!(buf = arr->_buf)) {
		static const MPT_STRUCT(type_traits) _values_double_info = MPT_TYPETRAIT_INIT(double, 'd');
		if (len < 0) {
			errno = EINVAL;
			return 0;
		}
		add = len * sizeof(double);
		if (!(buf = _mpt_buffer_alloc(add, 0))) {
			return 0;
		}
		arr->_buf = buf;
		
		buf->_typeinfo = &_values_double_info;
		buf->_used = add;
		
		return memset(buf + 1, 0, add);
	}
	/* use raw data buffer */
	if (!(info = buf->_typeinfo)
	    || info->type != 'd') {
		errno = EBADSLT;
		return 0;
	}
	/* existing size and limit */
	used = buf->_used;
	if (len >= 0) {
		add = len * sizeof(double);
	}
	else if (used < (-len)) {
		errno = EINVAL;
		return 0;
	} else {
		add = (-len) * sizeof(double);
	}
	if (!(buf = buf->_vptr->detach(buf, used + add))) {
		return 0;
	}
	arr->_buf = buf;
	data = (void *) (buf + 1);
	if (len < 0) {
		data = memcpy(data + used, data + (used - add), add);
	} else {
		data = memset(data + used, 0, add);
	}
	buf->_used = used + add;
	return (double *) data;
}

