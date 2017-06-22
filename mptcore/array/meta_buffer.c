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
	MPT_INTERFACE(metatype) _mt;
	MPT_INTERFACE(iterator) _it;
	MPT_STRUCT(slice) s;
};

static void bufferIterUnref(MPT_INTERFACE(unrefable) *ref)
{
	MPT_STRUCT(metaBuffer) *m = MPT_reladdr(metaBuffer, ref, _it, _mt);
	mpt_array_clone(&m->s._a, 0);
	free(m);
}
static int bufferGet(MPT_INTERFACE(iterator) *it, int type, void *ptr)
{
	MPT_STRUCT(metaBuffer) *m = MPT_reladdr(metaBuffer, it, _it, _mt);
	
	if (!type) {
		MPT_STRUCT(value) *val;
		if ((val = ptr)) {
			static const char fmt[] = { MPT_ENUM(TypeBuffer), 0 };
			val->fmt = fmt;
			val->ptr = &m->s._a;
		}
		return 0;
	}
	return mpt_slice_get(&m->s, type, ptr);
}
static int bufferAdvance(MPT_INTERFACE(iterator) *it)
{
	MPT_STRUCT(metaBuffer) *m = MPT_reladdr(metaBuffer, it, _it, _mt);
	const MPT_STRUCT(buffer) *buf;
	const char *base, *end;
	
	if (!(buf = m->s._a._buf)) {
		return MPT_ERROR(MissingData);
	}
	/* advance converted area */
	if (m->s._len) {
		m->s._off += m->s._len;
		m->s._len = 0;
		return 's';
	}
	/* find inline string separator */
	base = (void *) (buf + 1);
	if (!(end = memchr(base + m->s._off, 0, buf->_used - m->s._off))) {
		m->s._off = buf->_used;
		m->s._len = 0;
		return 0;
	}
	m->s._off += (end + 1) - base;
	m->s._len = 0;
	
	return 's';
}
static int bufferReset(MPT_INTERFACE(iterator) *it)
{
	MPT_STRUCT(metaBuffer) *m = MPT_reladdr(metaBuffer, it, _it, _mt);
	MPT_STRUCT(buffer) *buf;
	
	buf = m->s._a._buf;
	m->s._off = 0;
	m->s._len = buf ? buf->_used : 0;
	return 0;
}

static const MPT_INTERFACE_VPTR(iterator) _vptr_iter_buffer = {
	{ bufferIterUnref },
	bufferGet,
	bufferReset,
	bufferAdvance
};

static void bufferMetaUnref(MPT_INTERFACE(unrefable) *meta)
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
		static const char types[] = { MPT_ENUM(TypeIterator), MPT_value_toVector('c'), 's', 0 };
		if (dest) *dest = (void *) types;
		return 0;
	}
	if (type == MPT_ENUM(TypeIterator)) {
		if (dest) *dest = (void *) &m->_it;
		return MPT_ENUM(TypeIterator);
	}
	if (type == MPT_value_toVector('c')
	    || type == MPT_ENUM(TypeVector)) {
		struct iovec *vec;
		if ((vec = ptr)) {
			MPT_STRUCT(buffer) *buf;
			if ((buf = m->s._a._buf)) {
				vec->iov_base = buf + 1;
				vec->iov_len = buf->_used - m->s._off;
			} else {
				vec->iov_base = 0;
				vec->iov_len = 0;
			}
		}
		return MPT_ENUM(TypeBuffer);
	}
	return MPT_ERROR(BadType);
}
static const MPT_INTERFACE_VPTR(metatype) _vptr_meta_buffer;
static MPT_INTERFACE(metatype) *bufferClone(const MPT_INTERFACE(metatype) *meta)
{
	const MPT_STRUCT(metaBuffer) *old = (void *) meta;
	MPT_STRUCT(metaBuffer) *m;
	
	if (!(m = malloc(sizeof(*m)))) {
		return 0;
	}
	m->_mt._vptr = &_vptr_meta_buffer;
	m->_it._vptr = &_vptr_iter_buffer;
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
	return &m->_mt;
}
static const MPT_INTERFACE_VPTR(metatype) _vptr_meta_buffer = {
	{ bufferMetaUnref },
	bufferConv,
	bufferClone
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
	}
	return &m->_it;
}
