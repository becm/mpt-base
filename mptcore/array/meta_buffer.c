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
	MPT_INTERFACE(metatype) _meta;
	MPT_STRUCT(array)        arr;
};

static void bufferUnref(MPT_INTERFACE(metatype) *meta)
{
	MPT_STRUCT(metaBuffer) *m = (void *) meta;
	mpt_array_clone(&m->arr, 0);
	free(m);
}
static int bufferAssign(MPT_INTERFACE(metatype) *meta, const MPT_STRUCT(value) *vorg)
{
	MPT_STRUCT(metaBuffer) *m = (void *) meta;
	MPT_STRUCT(array) arr = MPT_ARRAY_INIT;
	MPT_STRUCT(value) val;
	
	if (!vorg) {
		return MPT_ENUM(TypeArrBase);
	}
	val = *vorg;
	
	if (!val.fmt) {
		size_t len = val.ptr ? strlen(val.ptr) + 1 : 0;
		
		if (!mpt_array_append(&arr, len, val.ptr)) {
			return MPT_ERROR(BadOperation);
		}
		mpt_array_clone(&m->arr, &arr);
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
	mpt_array_clone(&m->arr, &arr);
	mpt_array_clone(&arr, 0);
	
	return val.fmt - vorg->fmt;
}
static int bufferConv(MPT_INTERFACE(metatype) *meta, int type, void *ptr)
{
	MPT_STRUCT(metaBuffer) *m = (void *) meta;
	void **dest = ptr;
	
	if (type & MPT_ENUM(ValueConsume)) {
		return MPT_ERROR(BadArgument);
	}
	if (!type) {
		static const char types[] = { MPT_ENUM(TypeMeta), 'C', 's', 0 };
		if (dest) *dest = (void *) types;
		return 0;
	}
	switch (type) {
	  case MPT_ENUM(TypeMeta): ptr = &m->_meta; break;
	  case MPT_ENUM(TypeArrBase): ptr = &m->arr; break;
	  case 'C': ptr = &m->arr; break;
	  case 's': ptr = mpt_array_string(&m->arr); break;
	  default: return MPT_ERROR(BadType);
	}
	if (dest) *dest = ptr;
	return type;
}
static const MPT_INTERFACE_VPTR(metatype) _vptr_buffer;
static MPT_INTERFACE(metatype) *bufferClone(MPT_INTERFACE(metatype) *meta)
{
	MPT_STRUCT(metaBuffer) *m, *old = (void *) meta;
	
	if (!(m = malloc(sizeof(*m)))) {
		return 0;
	}
	m->_meta._vptr = &_vptr_buffer;
	m->arr._buf = 0;
	
	mpt_array_clone(&m->arr, &old->arr);
	
	return &m->_meta;
}

static const MPT_INTERFACE_VPTR(metatype) _vptr_buffer = {
	bufferUnref,
	bufferAssign,
	bufferConv,
	bufferClone
};

/*!
 * \ingroup mptArray
 * \brief create buffer metatype
 * 
 * Create metatype with dynamic data size.
 * 
 * \param name name of created metatype
 * \param len  initial data size
 * \param data initial buffer data
 * 
 * \return new metatype
 */
extern MPT_INTERFACE(metatype) *mpt_meta_buffer(size_t len, const void *data)
{
	MPT_STRUCT(metaBuffer) *mb;
	
	if (!(mb = malloc(sizeof(*mb)))) {
		return 0;
	}
	mb->_meta._vptr = &_vptr_buffer;
	mb->arr._buf = 0;
	
	if (!len || mpt_array_append(&mb->arr, len, data)) return &mb->_meta;
	
	free(mb);
	
	return 0;
}

