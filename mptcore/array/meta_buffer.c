/*!
 * \file
 * control handler for buffer storage.
 */

#include <string.h>
#include <stdlib.h>

#include <sys/uio.h>

#include "convert.h"
#include "array.h"

#include "meta.h"

MPT_STRUCT(metaBuffer) {
	MPT_INTERFACE(metatype) _meta;
	MPT_STRUCT(slice) s;
};

static void bufferUnref(MPT_INTERFACE(unrefable) *meta)
{
	MPT_STRUCT(metaBuffer) *m = (void *) meta;
	mpt_array_clone(&m->s._a, 0);
	free(m);
}
static int bufferAssign(MPT_INTERFACE(metatype) *meta, const MPT_STRUCT(value) *vorg)
{
	MPT_STRUCT(metaBuffer) *m = (void *) meta;
	MPT_STRUCT(array) arr = MPT_ARRAY_INIT;
	MPT_STRUCT(value) val;
	
	if (!vorg) {
		m->s._len = m->s._a._buf ? m->s._a._buf->used : 0;
		m->s._off = 0;
		return MPT_ENUM(TypeArrBase);
	}
	val = *vorg;
	
	if (!val.fmt) {
		size_t len;
		
		if (!val.ptr) {
			mpt_array_clone(&m->s._a, 0);
			m->s._off = m->s._len = 0;
			return 0;
		}
		len = strlen(val.ptr) + 1;
		if (!mpt_array_append(&arr, len, val.ptr)) {
			return MPT_ERROR(BadOperation);
		}
		mpt_array_clone(&m->s._a, &arr);
		m->s._len = m->s._a._buf->used;
		m->s._off = 0;
		
		mpt_array_clone(&arr, 0);
		
		return len;
	}
	while (*val.fmt) {
		const char *base;
		size_t len;
		
		if (!(base = mpt_data_tostring(&val.ptr, *val.fmt++, &len))) {
			return MPT_ERROR(BadValue);
		}
		if (len && !mpt_array_append(&arr, len, base)) {
			return MPT_ERROR(BadOperation);
		}
		if (!mpt_array_append(&arr, 1, 0)) {
			return MPT_ERROR(BadOperation);
		}
	}
	mpt_array_clone(&m->s._a, &arr);
	m->s._len = m->s._a._buf ? m->s._a._buf->used : 0;
	m->s._off = 0;
	
	mpt_array_clone(&arr, 0);
	
	return val.fmt - vorg->fmt;
}
static int bufferConv(MPT_INTERFACE(metatype) *meta, int type, void *ptr)
{
	MPT_STRUCT(metaBuffer) *m = (void *) meta;
	void **dest = ptr;
	
	if (!type) {
		static const char types[] = { MPT_ENUM(TypeMeta), MPT_value_toArray('c'), 's', 0 };
		if (dest) *dest = (void *) types;
		return 0;
	}
	if ((type & 0xff) == MPT_ENUM(TypeMeta)) {
		if (type & MPT_ENUM(ValueConsume)) {
			return MPT_ERROR(BadArgument);
		}
		if (dest) *dest = &m->_meta;
		return MPT_ENUM(TypeMeta);
	}
	if ((type & 0xff) == MPT_value_toArray('c')) {
		if (type & MPT_ENUM(ValueConsume)) {
			return MPT_ERROR(BadArgument);
		}
		if (dest) *dest = &m->s._a;
		return MPT_value_toArray('c');
	}
	return mpt_slice_conv(&m->s, type, ptr);
}
static const MPT_INTERFACE_VPTR(metatype) _vptr_buffer;
static MPT_INTERFACE(metatype) *bufferClone(const MPT_INTERFACE(metatype) *meta)
{
	const MPT_STRUCT(metaBuffer) *old = (void *) meta;
	MPT_STRUCT(metaBuffer) *m;
	
	if (!(m = malloc(sizeof(*m)))) {
		return 0;
	}
	m->_meta._vptr = &_vptr_buffer;
	m->s._a._buf = 0;
	
	if (old) {
		mpt_array_clone(&m->s._a, &old->s._a);
		if (m->s._a._buf) {
			m->s._len = old->s._len;
			m->s._off = old->s._off;
			return &m->_meta;
		}
	}
	m->s._len = 0;
	m->s._off = 0;
	
	return &m->_meta;
}

static const MPT_INTERFACE_VPTR(metatype) _vptr_buffer = {
	{ bufferUnref },
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
extern MPT_INTERFACE(metatype) *mpt_meta_buffer(const MPT_STRUCT(array) *a)
{
	MPT_STRUCT(metaBuffer) *m;
	
	if (!(m = (void *) bufferClone(0))) {
		return 0;
	}
	if (a) {
		mpt_array_clone(&m->s._a, a);
		if (m->s._a._buf) m->s._len = m->s._a._buf->used;
	}
	return &m->_meta;
}
