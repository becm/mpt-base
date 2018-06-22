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

/* iterator interface */
static int bufferGet(MPT_INTERFACE(iterator) *it, int type, void *ptr)
{
	MPT_STRUCT(metaBuffer) *m = MPT_baseaddr(metaBuffer, it, _it);
	return mpt_slice_get(&m->s, type, ptr);
}
static int bufferAdvance(MPT_INTERFACE(iterator) *it)
{
	MPT_STRUCT(metaBuffer) *m = MPT_baseaddr(metaBuffer, it, _it);
	return mpt_slice_advance(&m->s);
}
static int bufferReset(MPT_INTERFACE(iterator) *it)
{
	MPT_STRUCT(metaBuffer) *m = MPT_baseaddr(metaBuffer, it, _it);
	MPT_STRUCT(buffer) *buf;
	
	buf = m->s._a._buf;
	m->s._off = 0;
	m->s._len = buf ? buf->_used : 0;
	return 0;
}
/* reference interface */
static void bufferUnref(MPT_INTERFACE(reference) *ref)
{
	MPT_STRUCT(metaBuffer) *m = MPT_baseaddr(metaBuffer, ref, _mt);
	mpt_array_clone(&m->s._a, 0);
	free(m);
}
static uintptr_t bufferRef(MPT_INTERFACE(reference) *ref)
{
	(void) ref;
	return 0;
}
/* metatype interface */
static int bufferConv(const MPT_INTERFACE(metatype) *mt, int type, void *ptr)
{
	const MPT_STRUCT(metaBuffer) *m = (void *) mt;
	
	if (!type) {
		static const uint8_t fmt[] = { MPT_ENUM(TypeIterator), MPT_type_vector('c'), 0 };
		if (ptr) {
			*((const uint8_t **) ptr) = fmt;
			return MPT_ENUM(TypeArray);
		}
		return MPT_ENUM(TypeIterator);
	}
	if (type == MPT_type_pointer(MPT_ENUM(TypeIterator))) {
		if (ptr) *((const void **) ptr) = &m->_it;
		return MPT_ENUM(TypeArray);
	}
	if (type == MPT_type_vector('c')
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
		return MPT_ENUM(TypeIterator);
	}
	return MPT_ERROR(BadType);
}
static MPT_INTERFACE(metatype) *bufferCopy(MPT_INTERFACE(metatype) *(*copy)(const MPT_STRUCT(array) *), const MPT_STRUCT(slice) *sl)
{
	MPT_STRUCT(metaBuffer) *ptr;
	MPT_INTERFACE(metatype) *res;
	if (!(res = copy(&sl->_a))) {
		return 0;
	}
	ptr = (void *) res;
	ptr->s._len = sl->_len;
	ptr->s._off = sl->_off;
	return res;
}
static MPT_INTERFACE(metatype) *bufferClone(const MPT_INTERFACE(metatype) *mt)
{
	const MPT_STRUCT(metaBuffer) *m = (void *) mt;
	return bufferCopy(mpt_meta_buffer, &m->s);
}

/*!
 * \ingroup mptArray
 * \brief create buffer iterator
 * 
 * Create text iterator with dynamic data size.
 * 
 * \param a  array data to use for iterator
 * 
 * \return new iterator metatype
 */
extern MPT_INTERFACE(metatype) *mpt_meta_buffer(const MPT_STRUCT(array) *a)
{
	static const MPT_INTERFACE_VPTR(metatype) metaBuffer = {
		{ bufferUnref, bufferRef },
		bufferConv,
		bufferClone
	};
	static const MPT_INTERFACE_VPTR(iterator) iterBuffer = {
		bufferGet,
		bufferAdvance,
		bufferReset
	};
	static const MPT_STRUCT(slice) s = MPT_SLICE_INIT;
	MPT_STRUCT(metaBuffer) *m;
	
	if (!(m = malloc(sizeof(*m)))) {
		return 0;
	}
	m->_mt._vptr = &metaBuffer;
	m->_it._vptr = &iterBuffer;
	m->s = s;
	
	if (a) {
		mpt_array_clone(&m->s._a, a);
	}
	return &m->_mt;
}
/* return first argument as string content */
static int bufferResetArgs(MPT_INTERFACE(iterator) *it)
{
	MPT_STRUCT(metaBuffer) *m = MPT_baseaddr(metaBuffer, it, _it);
	
	m->s._off = 0;
	m->s._len = 0;
	mpt_slice_advance(&m->s);
	return 0;
}
static int bufferConvArgs(const MPT_INTERFACE(metatype) *mt, int type, void *ptr)
{
	const MPT_STRUCT(metaBuffer) *m = (void *) mt;
	
	if (!type) {
		static const uint8_t fmt[] = { MPT_ENUM(TypeIterator), 's', 0 };
		if (ptr) {
			*((const uint8_t **) ptr) = fmt;
			return MPT_ENUM(TypeArray);
		}
		return MPT_ENUM(TypeIterator);
	}
	if (type == 's') {
		if (ptr) {
			MPT_STRUCT(buffer) *buf = m->s._a._buf;
			if (buf && m->s._off) {
				++buf;
			}
			*((const void **) ptr) = buf;
		}
		return MPT_ENUM(TypeIterator);
	}
	if (type == MPT_type_pointer(MPT_ENUM(TypeIterator))) {
		if (ptr) {
			MPT_STRUCT(buffer) *buf = m->s._a._buf;
			if (buf && m->s._off < buf->_used) {
				*((const void **) ptr) = &m->_it;
			} else {
				*((const void **) ptr) = 0;
			}
		}
		return MPT_ENUM(TypeArray);
	}
	return MPT_ERROR(BadType);
}
static MPT_INTERFACE(metatype) *bufferCloneArgs(const MPT_INTERFACE(metatype) *mt)
{
	const MPT_STRUCT(metaBuffer) *m = (void *) mt;
	MPT_STRUCT(metaBuffer) *ptr;
	void *copy;
	if (!(copy = mpt_meta_arguments(&m->s._a))) {
		return 0;
	}
	ptr = copy;
	ptr->s._len = m->s._len;
	ptr->s._off = m->s._off;
	return &ptr->_mt;
}
/*!
 * \ingroup mptArray
 * \brief create argument iterator
 * 
 * Use first string segment as metatype value.
 * Supply text iterator for subsequent elements.
 * 
 * \param a  array data to use for iterator
 * 
 * \return new iterator metatype
 */
extern MPT_INTERFACE(metatype) *mpt_meta_arguments(const MPT_STRUCT(array) *a)
{
	static const MPT_INTERFACE_VPTR(metatype) metaBuffer = {
		{ bufferUnref, bufferRef },
		bufferConvArgs,
		bufferCloneArgs
	};
	static const MPT_INTERFACE_VPTR(iterator) iterBuffer = {
		bufferGet,
		bufferAdvance,
		bufferResetArgs
	};
	static const MPT_STRUCT(slice) s = MPT_SLICE_INIT;
	MPT_INTERFACE(metaBuffer) *m;
	
	if (!(m = malloc(sizeof(*m)))) {
		return 0;
	}
	m->_mt._vptr = &metaBuffer;
	m->_it._vptr = &iterBuffer;
	m->s = s;
	
	if (a) {
		mpt_array_clone(&m->s._a, a);
		mpt_slice_advance(&m->s);
	}
	
	return &m->_mt;
}
