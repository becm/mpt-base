/*!
 * get raw data type and data area.
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "values.h"

static int _value_store_type = 0;

static void _value_store_init(const MPT_STRUCT(type_traits) *info, void *ptr)
{
	memset(ptr, 0, info->size);
}

static int _value_store_copy(void *src, int type, void *dest)
{
	const MPT_STRUCT(value_store) *from = src;
	MPT_STRUCT(value_store) *to = dest;
	
	if (type == MPT_ENUM(TypeArray)) {
		return mpt_array_clone(&to->_d, &from->_d);
	}
	if (type != _value_store_type) {
		return MPT_ERROR(BadType);
	}
	if ((type = mpt_array_clone(&to->_d, &from->_d)) < 0) {
		return type;
	}
	to->_type   = from->_type;
	to->_esize  = from->_esize;
	to->_flags  = from->_flags;
	
	return 0;
}
static void _value_store_fini(void *ptr)
{
	MPT_STRUCT(value_store) *val = ptr;
	mpt_array_clone(&val->_d, 0);
}

/*!
 * \ingroup mptPlot
 * \brief get/create stage dimension
 * 
 * Set flags and get target array for raw data dimension.
 * 
 * \param st   raw data stage data
 * \param dim  dimension to process
 * 
 * \return dimension data array
 */
extern MPT_STRUCT(value_store) *mpt_stage_data(MPT_STRUCT(rawdata_stage) *st, unsigned dim)
{
	MPT_STRUCT(buffer) *buf = st->_d._buf;
	MPT_STRUCT(value_store) *val;
	unsigned max = 0;
	
	if (!_value_store_type) {
		_value_store_type = mpt_value_store_typeid();
	}
	if (_value_store_type < 0) {
		errno = EBADSLT;
		return 0;
	}
	
	/* existing buffer must be valied */
	if (buf) {
		const MPT_STRUCT(type_traits) *info;
		if (!(info = buf->_typeinfo)
		    || info->type != _value_store_type) {
			errno = EBADSLT;
			return 0;
		}
		max = buf->_used / sizeof(*val);
	}
	/* not allowed to extend dimensions */
	if (st->_max_dimensions
	    && (dim >= st->_max_dimensions)) {
		errno = ENOMEM;
		return 0;
	}
	if (dim >= max) {
		max = dim + 1;
	}
	max *= sizeof(*val);
	if (!buf) {
		MPT_STRUCT(type_traits) info = MPT_TYPETRAIT_INIT(*val, MPT_ENUM(TypeArray));
		
		info.init = _value_store_init;
		info.copy = _value_store_copy;
		info.fini = _value_store_fini;
		info.type = _value_store_type;
		
		if (!(buf = _mpt_buffer_alloc(max, &info))) {
			return 0;
		}
		val = memset(buf + 1, 0, max);
		return val + dim;
	}
	if (!(buf = buf->_vptr->detach(buf, max))) {
		return 0;
	}
	st->_d._buf = buf;
	
	/* init function executed on insert */
	val = mpt_buffer_insert(buf, dim * sizeof(*val), sizeof(*val));
	return val;
}
