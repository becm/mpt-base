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
	MPT_INTERFACE(iterator) _it;
	MPT_STRUCT(slice) s;
};

static void bufferUnref(MPT_INTERFACE(unrefable) *meta)
{
	MPT_STRUCT(metaBuffer) *m = (void *) meta;
	mpt_array_clone(&m->s._a, 0);
	free(m);
}
static int bufferConv(const MPT_INTERFACE(metatype) *meta, int type, void *ptr)
{
	const MPT_STRUCT(metaBuffer) *m = (void *) meta;
	void **dest = ptr;
	
	if (!type) {
		static const char types[] = { MPT_ENUM(TypeMeta), MPT_value_toArray('c'), 's', 0 };
		if (dest) *dest = (void *) types;
		return 0;
	}
	if (type == MPT_ENUM(TypeMeta)
	    || type == MPT_ENUM(TypeIterator)) {
		if (dest) *dest = (void *) &m->_it;
		return MPT_ENUM(TypeIterator);
	}
	if (type == MPT_value_toArray('c')
	    || type == MPT_ENUM(TypeArrBase)) {
		if (m->s._off) {
			return MPT_ERROR(BadArgument);
		}
	}
	return mpt_slice_conv(&m->s, type, ptr);
}
static int bufferAdvance(MPT_INTERFACE(iterator) *it)
{
	MPT_STRUCT(metaBuffer) *m = (void *) it;
	const MPT_STRUCT(buffer) *buf;
	const char *base, *end;
	size_t len;
	
	if (!(buf = m->s._a._buf) || !m->s._len) {
		return MPT_ERROR(MissingData);
	}
	base = (void *) (buf + 1);
	if (!(end = memchr(base + m->s._off, 0, m->s._len))) {
		return MPT_ERROR(MissingData);
	}
	len = (end + 1) - base;
	
	m->s._off += len;
	m->s._len -= len;
	
	return len;
}
static int bufferReset(MPT_INTERFACE(iterator) *it)
{
	MPT_STRUCT(metaBuffer) *m = (void *) it;
	MPT_STRUCT(buffer) *buf;
	
	buf = m->s._a._buf;
	m->s._off = 0;
	m->s._len = buf ? buf->used : 0;
	return MPT_value_toArray('c');
}
static const MPT_INTERFACE_VPTR(iterator) _vptr_buffer;
static MPT_INTERFACE(metatype) *bufferClone(const MPT_INTERFACE(metatype) *meta)
{
	const MPT_STRUCT(metaBuffer) *old = (void *) meta;
	MPT_STRUCT(metaBuffer) *m;
	
	if (!(m = malloc(sizeof(*m)))) {
		return 0;
	}
	m->_it._vptr = &_vptr_buffer;
	m->s._a._buf = 0;
	m->s._len = 0;
	m->s._off = 0;
	
	if (old) {
		mpt_array_clone(&m->s._a, &old->s._a);
		if (m->s._a._buf) {
			m->s._len = old->s._len;
			m->s._off = old->s._off;
		}
	}
	return (void *) &m->_it;
}

static const MPT_INTERFACE_VPTR(iterator) _vptr_buffer = {
	{ { bufferUnref }, bufferConv, bufferClone },
	bufferReset,
	bufferAdvance
};

/*!
 * \ingroup mptArray
 * \brief create buffer iterator
 * 
 * Create text iterator with dynamic data size.
 * 
 * \param a  array data to use for iterator
 * 
 * \return new iterator
 */
extern MPT_INTERFACE(iterator) *mpt_meta_buffer(const MPT_STRUCT(array) *a)
{
	MPT_STRUCT(metaBuffer) *m;
	
	if (!(m = (void *) bufferClone(0))) {
		return 0;
	}
	if (a) {
		mpt_array_clone(&m->s._a, a);
		if (m->s._a._buf) m->s._len = m->s._a._buf->used;
	}
	return &m->_it;
}
