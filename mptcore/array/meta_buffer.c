/*!
 * \file
 * control handler for buffer storage.
 */

#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

#include <sys/uio.h>

#include "convert.h"

#include "array.h"

MPT_STRUCT(metaBuffer) {
	MPT_STRUCT(array)        arr;
	MPT_INTERFACE(metatype) _meta;
	uint64_t                _info;
};

static int bufferUnref(MPT_INTERFACE(metatype) *meta)
{
	MPT_STRUCT(metaBuffer) *mb = MPT_reladdr(metaBuffer, meta, _meta, arr);
	int c = _mpt_geninfo_unref(&mb->_info);
	if (c) return c;
	mpt_array_clone(&mb->arr, 0);
	free(mb);
	return 0;
}
static MPT_INTERFACE(metatype) *bufferAddref(MPT_INTERFACE(metatype) *meta)
{
	MPT_STRUCT(metaBuffer) *mb = MPT_reladdr(metaBuffer, meta, _meta, arr);
	return (_mpt_geninfo_addref(&mb->_info)) ? meta : 0;
}
static int bufferAssign(MPT_INTERFACE(metatype) *meta, const MPT_STRUCT(value) *vorg)
{
	MPT_STRUCT(metaBuffer) *mb = MPT_reladdr(metaBuffer, meta, _meta, arr);
	MPT_STRUCT(array) arr = MPT_ARRAY_INIT;
	MPT_STRUCT(value) val;
	
	if (!vorg) {
		return MPT_ENUM(TypeArray);
	}
	val = *vorg;
	
	if (!val.fmt) {
		size_t len = val.ptr ? strlen(val.ptr) + 1 : 0;
		
		if (!mpt_array_append(&arr, len, val.ptr)) {
			return MPT_ERROR(BadOperation);
		}
		mpt_array_clone(&mb->arr, &arr);
		mpt_array_clone(&arr, 0);
		return len;
	}
	while (*val.fmt) {
		const char *base;
		size_t len;
		
		if (!(base = mpt_data_tostring(&val.ptr, *val.fmt, &len))) {
			return MPT_ERROR(BadValue);
		}
		if (!mpt_array_append(&arr, len, base)) {
			return MPT_ERROR(BadOperation);
		}
		++val.fmt;
	}
	mpt_array_clone(&mb->arr, &arr);
	mpt_array_clone(&arr, 0);
	
	return val.fmt - vorg->fmt;
}
static void *bufferCast(MPT_INTERFACE(metatype) *meta, int type)
{
	MPT_STRUCT(metaBuffer) *mb = MPT_reladdr(metaBuffer, meta, _meta, arr);
	
	switch (type) {
	  case MPT_ENUM(TypeMeta): return meta;
	  case MPT_ENUM(TypeArray): return &mb->arr;
	  case 's': return mpt_array_string(&mb->arr);
	  default: return 0;
	}
}

static const MPT_INTERFACE_VPTR(metatype) _vptr_buffer = {
	bufferUnref,
	bufferAddref,
	bufferAssign,
	bufferCast
};

/*!
 * \ingroup mptArray
 * \brief create buffer metatype
 * 
 * Create metatype with dynamic data size.
 * 
 * \param name	name of created metatype
 * \param len	initial data size
 * \param data	initial buffer data
 * 
 * \return new metatype
 */
extern MPT_INTERFACE(metatype) *mpt_meta_buffer(size_t len, const void *data)
{
	MPT_STRUCT(metaBuffer) *mb;
	
	if (!(mb = malloc(sizeof(*mb)))) {
		return 0;
	}
	if (_mpt_geninfo_init(&mb->_info, sizeof(mb->_info), 1) < 0) {
		free(mb);
		return 0;
	}
	
	mb->arr._buf = 0;
	mb->_meta._vptr = &_vptr_buffer;
	
	if (!len || mpt_array_append(&mb->arr, len, data)) return &mb->_meta;
	
	free(mb);
	
	return 0;
}

