/*!
 * get raw data type and data area.
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "values.h"

/* reference interface */
static void stageDataUnref(MPT_INTERFACE(reference) *ref)
{
	MPT_STRUCT(buffer) *buf = (void *) ref;
	
	if (!mpt_refcount_lower(&buf->_ref)) {
		MPT_STRUCT(value_store) *val = (void *) (buf + 1);
		size_t i, len = buf->_used / sizeof(*val);
		for (i = 0; i < len; ++i) {
			mpt_array_clone(&val->_d, 0);
		}
		free(buf);
	}
}
static uintptr_t stageDataRef(MPT_INTERFACE(reference) *ref)
{
	MPT_STRUCT(buffer) *buf = (void *) ref;
	
	return mpt_refcount_raise(&buf->_ref);
}
static MPT_INTERFACE_VPTR(buffer) stageData;
static MPT_STRUCT(buffer) *stageDataDetach(MPT_STRUCT(buffer) *buf, long len)
{
	MPT_STRUCT(value_store) *val;
	
	if (len < 0) {
		len = buf->_used / sizeof(*val);
	}
	/* require reference copies */
	if (buf->_ref._val > 1) {
		MPT_STRUCT(value_store) *ptr;
		MPT_STRUCT(buffer) *b;
		long i = len * sizeof(*val);
		
		if (!(b = malloc(sizeof(*buf) + i))) {
			return 0;
		}
		b->_vptr = &stageData;
		b->_ref._val = 1;
		b->_size = i;
		b->_used = 0;
		
		val = (void *) (buf + 1);
		ptr = (void *) (b + 1);
		len = buf->_used / sizeof(*val);
		for (i = 0; i < len; ++i) {
			MPT_STRUCT(buffer) *tmp = val[i]._d._buf;
			if (tmp) mpt_refcount_raise(&tmp->_ref);
			ptr[i] = val[i];
		}
		b->_used = len * sizeof(*ptr);
		mpt_refcount_lower(&buf->_ref);
		return b;
	}
	if ((size_t) len <= buf->_size / sizeof(*val)) {
		long i, max = buf->_used / sizeof(*val);
		val = (void *) (buf + 1);
		for (i = max; i < len; ++i) {
			memset(val + i, 0, sizeof(*val));
		}
		for (i = len; i < max; ++i) {
			MPT_STRUCT(buffer) *tmp = val[i]._d._buf;
			if (tmp) tmp->_vptr->ref.unref((void *) tmp);
		}
		buf->_used = len * sizeof(*val);
		return buf;
	}
	len *= sizeof(*val);
	if ((buf = realloc(buf, sizeof(*buf) + len))) {
		buf->_size = len;
	}
	return buf;
}
static int stageDataType(const MPT_STRUCT(buffer) *buf)
{
	(void) buf;
	return MPT_ENUM(TypeArray);
}
static MPT_INTERFACE_VPTR(buffer) stageDataCtl = {
	{ stageDataUnref, stageDataRef },
	stageDataDetach,
	stageDataType
};

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
	
	if (buf) {
		if (buf->_vptr->content(buf) == stageDataType(0)) {
			/* return existing slot */
			if (buf->_ref._val < 2
			    && (max = buf->_used / sizeof(*val)) > dim) {
				val = (void *) (buf + 1);
				return val + dim;
			}
		}
		/* invalidate current buffer */
		else {
			buf->_vptr->ref.unref((void *) buf);
			st->_d._buf = buf = 0;
		}
	}
	/* not allowed to extend dimensions */
	if (st->_max_dimensions && (dim >= st->_max_dimensions)) {
		errno = EINVAL;
		return 0;
	}
	if (dim >= max) {
		max = dim + 1;
	}
	if (!buf) {
		size_t len = max * sizeof(*val);
		if (!(buf = malloc(sizeof(*buf) + len))) {
			return 0;
		}
		buf->_vptr = &stageDataCtl;
		buf->_ref._val = 1;
		buf->_size = len;
		buf->_used = 0;
	}
	else if (!(buf = buf->_vptr->detach(buf, max))) {
		return 0;
	}
	st->_d._buf = buf;
	
	if (!(val = mpt_buffer_insert(buf, dim * sizeof(*val), sizeof(*val)))) {
		return val;
	}
	val->_d._buf = 0;
	val->_type   = 0;
	val->_esize  = 0;
	val->_flags  = 0;
	
	return val;
}
