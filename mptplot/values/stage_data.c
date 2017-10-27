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
		MPT_STRUCT(typed_array) *arr = (void *) (buf + 1);
		size_t i, len = buf->_used / sizeof(*arr);
		for (i = 0; i < len; ++i) {
			mpt_array_clone(&arr->_d, 0);
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
	MPT_STRUCT(typed_array) *arr;
	
	if (len < 0) {
		len = buf->_used / sizeof(*arr);
	}
	/* require reference copies */
	if (buf->_ref._val > 1) {
		MPT_STRUCT(typed_array) *ptr;
		MPT_STRUCT(buffer) *b;
		long i = len * sizeof(*arr);
		
		if (!(b = malloc(sizeof(*buf) + i))) {
			return 0;
		}
		b->_vptr = &stageData;
		b->_ref._val = 1;
		b->_size = i;
		b->_used = 0;
		
		arr = (void *) (buf + 1);
		ptr = (void *) (b + 1);
		len = buf->_used / sizeof(*arr);
		for (i = 0; i < len; ++i) {
			MPT_STRUCT(buffer) *tmp = arr[i]._d._buf;
			if (tmp) mpt_refcount_raise(&tmp->_ref);
			ptr[i] = arr[i];
		}
		b->_used = len * sizeof(*ptr);
		mpt_refcount_lower(&buf->_ref);
		return b;
	}
	if ((size_t) len <= buf->_size / sizeof(*arr)) {
		long i, max = buf->_used / sizeof(*arr);
		arr = (void *) (buf + 1);
		for (i = max; i < len; ++i) {
			memset(arr + i, 0, sizeof(*arr));
		}
		for (i = len; i < max; ++i) {
			MPT_STRUCT(buffer) *tmp = arr[i]._d._buf;
			if (tmp) tmp->_vptr->ref.unref((void *) tmp);
		}
		buf->_used = len * sizeof(*arr);
		return buf;
	}
	len *= sizeof(*arr);
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
extern MPT_STRUCT(typed_array) *mpt_stage_data(MPT_STRUCT(rawdata_stage) *st, unsigned dim)
{
	MPT_STRUCT(buffer) *buf = st->_d._buf;
	MPT_STRUCT(typed_array) *arr;
	size_t len;
	unsigned max = 0;
	
	if (buf) {
		if (buf->_vptr->content(buf) == stageDataType(0)) {
			/* return existing slot */
			if (buf->_ref._val < 2
			    && (max = buf->_used / sizeof(*arr)) > dim) {
				arr = (void *) (buf + 1);
				return arr + dim;
			}
		}
		/* invalidate current buffer */
		else {
			buf->_vptr->ref.unref((void *) buf);
			st->_d._buf = buf = 0;
		}
	}
	/* not allowed to extend dimensions */
	if (st->_maxDimensions && (dim >= st->_maxDimensions)) {
		errno = EINVAL;
		return 0;
	}
	if (dim >= max) max = dim + 1;
	len = max * sizeof(*arr);
	if (!buf) {
		if (!(buf = malloc(sizeof(*buf) + len))) {
			return 0;
		}
		buf->_vptr = &stageDataCtl;
		buf->_ref._val = 1;
		buf->_size = len;
		buf->_used = 0;
	}
	else if (!(buf = buf->_vptr->detach(buf, len))) {
		return 0;
	}
	st->_d._buf = buf;
	
	if (!(arr = mpt_buffer_insert(buf, dim * sizeof(*arr), sizeof(*arr)))) {
		return arr;
	}
	arr->_d._buf = 0;
	arr->_type   = 0;
	arr->_esize  = 0;
	arr->_flags  = 0;
	
	return arr;
}
