/*!
 * \file
 * control handler for buffer storage.
 */

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/uio.h>

#include "convert.h"
#include "array.h"
#include "types.h"

#include "meta.h"

MPT_STRUCT(metaBuffer) {
	MPT_INTERFACE(metatype) _mt;
	MPT_INTERFACE(iterator) _it;
	
	struct {
		MPT_INTERFACE(convertable) _conv;
		const MPT_STRUCT(buffer) *match;
		MPT_TYPE(data_converter) converter;
		MPT_STRUCT(value) val;
	} entry;
	
	union {
		MPT_INTERFACE(convertable) *conv;
		struct iovec vec;
	} storage;
	const char *str;
	
	MPT_STRUCT(slice) s;
};

/* element convertable interface */
static int bufferConvertEntry(MPT_INTERFACE(convertable) *conv, MPT_TYPE(type) type, void *ptr)
{
	MPT_STRUCT(metaBuffer) *m = MPT_baseaddr(metaBuffer, conv, entry._conv);
	const MPT_STRUCT(buffer) *buf;
	uint8_t *base;
	
	if (!(buf = m->s._a._buf) || !m->s._len) {
		return MPT_ERROR(MissingData);
	}
	if (buf != m->entry.match) {
		return MPT_ERROR(BadOperation);
	}
	base = (void *) (buf + 1);
	
	return m->entry.converter(base + m->s._off, type, ptr);
}
/* iterator interface */
static const MPT_STRUCT(value) *bufferGet(MPT_INTERFACE(iterator) *it)
{
	MPT_STRUCT(metaBuffer) *m = MPT_baseaddr(metaBuffer, it, _it);
	const MPT_STRUCT(buffer) *buf;
	
	if (!(buf = m->s._a._buf) || !m->s._len) {
		return 0;
	}
	if (buf != m->entry.match) {
		return 0;
	}
	if (!m->str) {
		uint8_t *data = (void *) (buf + 1);
		m->storage.vec.iov_base = data + m->s._off;
		m->storage.vec.iov_len  = m->s._len;
		MPT_value_set(&m->entry.val, MPT_type_toVector('c'), &m->storage.vec);
	}
	else if (m->entry.converter) {
		m->storage.conv = &m->entry._conv;
		MPT_value_set(&m->entry.val, MPT_ENUM(TypeConvertablePtr), &m->storage.conv);
	}
	else {
		MPT_value_set(&m->entry.val, 's', &m->str);
	}
	
	return &m->entry.val;
}
static int bufferAdvance(MPT_INTERFACE(iterator) *it)
{
	MPT_STRUCT(metaBuffer) *m = MPT_baseaddr(metaBuffer, it, _it);
	const MPT_STRUCT(buffer) *buf;
	int type;
	
	m->str = 0;
	if (!(buf = m->s._a._buf)) {
		return MPT_ERROR(MissingData);
	}
	if (buf != m->entry.match) {
		return MPT_ERROR(BadOperation);
	}
	if ((type = mpt_slice_next(&m->s)) <= 0) {
		return type;
	}
	if (type == 's') {
		char *data = (void *) (buf + 1);
		m->str = data + m->s._off;
	}
	return type;
}
static int bufferReset(MPT_INTERFACE(iterator) *it)
{
	MPT_STRUCT(metaBuffer) *m = MPT_baseaddr(metaBuffer, it, _it);
	int type;
	
	m->entry.match = m->s._a._buf;
	m->s._off = 0;
	m->s._len = 0;
	type = bufferAdvance(&m->_it);
	return (type < 0) ? 0 : type;
}
/* convertable interface */
static int bufferConv(MPT_INTERFACE(convertable) *val, MPT_TYPE(type) type, void *ptr)
{
	const MPT_STRUCT(metaBuffer) *m = (void *) val;
	
	if (!type) {
		if (ptr) {
			static const uint8_t fmt[] = {
				MPT_ENUM(TypeIteratorPtr),
				MPT_ENUM(TypeBufferPtr),
				MPT_type_toVector('c'),
				0
			};
			*((const uint8_t **) ptr) = fmt;
		}
		return MPT_ENUM(TypeMetaPtr);
	}
	if (type == MPT_ENUM(TypeMetaPtr)) {
		if (ptr) *((const void **) ptr) = &m->_mt;
		return MPT_ENUM(TypeArray);
	}
	if (type == MPT_ENUM(TypeIteratorPtr)) {
		if (ptr) *((const void **) ptr) = &m->_it;
		return MPT_ENUM(TypeArray);
	}
	if (type == MPT_ENUM(TypeBufferPtr)) {
		if (ptr) *((const void **) ptr) = m->s._a._buf;
		return MPT_ENUM(TypeArray);
	}
	if (type == MPT_type_toVector('c')
	    || type == MPT_ENUM(TypeVector)) {
		struct iovec *vec;
		if ((vec = ptr)) {
			MPT_STRUCT(buffer) *buf;
			if ((buf = m->s._a._buf)) {
				vec->iov_base = buf + 1;
				vec->iov_len = buf->_used;
			} else {
				vec->iov_base = 0;
				vec->iov_len = 0;
			}
		}
		return MPT_ENUM(TypeIteratorPtr);
	}
	return MPT_ERROR(BadType);
}
/* metatype interface */
static void bufferUnref(MPT_INTERFACE(metatype) *mt)
{
	MPT_STRUCT(metaBuffer) *m = MPT_baseaddr(metaBuffer, mt, _mt);
	mpt_array_clone(&m->s._a, 0);
	free(m);
}
static uintptr_t bufferRef(MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
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
		{ bufferConv },
		bufferUnref,
		bufferRef,
		bufferClone
	};
	static const MPT_INTERFACE_VPTR(iterator) iterBuffer = {
		bufferGet,
		bufferAdvance,
		bufferReset
	};
	static const MPT_INTERFACE_VPTR(convertable) convBufferEntry = {
		bufferConvertEntry
	};
	static const MPT_STRUCT(slice) s = MPT_SLICE_INIT;
	MPT_STRUCT(metaBuffer) *m;
	
	if (!(m = malloc(sizeof(*m)))) {
		return 0;
	}
	m->_mt._vptr = &metaBuffer;
	m->_it._vptr = &iterBuffer;
	
	m->entry._conv._vptr = &convBufferEntry;
	m->entry.match = 0;
	m->entry.converter = 0;
	
	MPT_value_set(&m->entry.val, MPT_ENUM(TypeConvertablePtr), &m->storage.conv);
	m->storage.conv = &m->entry._conv;
	
	m->s = s;
	if (a) {
		mpt_array_clone(&m->s._a, a);
		if (bufferReset(&m->_it) < 0) {
			bufferUnref(&m->_mt);
			errno = EINVAL;
			return 0;
		}
	}
	m->entry.match = m->s._a._buf;
	return &m->_mt;
}
/* skip first argument (convert to string content) */
static int bufferResetArgs(MPT_INTERFACE(iterator) *it)
{
	MPT_STRUCT(metaBuffer) *m = MPT_baseaddr(metaBuffer, it, _it);
	int ret;
	if ((ret = bufferReset(&m->_it)) <= 0) {
		return ret;
	}
	/* consume first argument of iterator */
	return bufferAdvance(&m->_it);
}
static int bufferConvArgs(MPT_INTERFACE(convertable) *val, MPT_TYPE(type) type, void *ptr)
{
	const MPT_STRUCT(metaBuffer) *m = (void *) val;
	
	if (!type) {
		if (ptr) {
			static const uint8_t fmt[] = {
				MPT_ENUM(TypeIteratorPtr),
				's',
				0
			};
			*((const uint8_t **) ptr) = fmt;
		}
		return MPT_ENUM(TypeMetaPtr);
	}
	if (type == MPT_ENUM(TypeMetaPtr)) {
		if (ptr) *((const void **) ptr) = &m->_mt;
		return MPT_ENUM(TypeArray);
	}
	if (type == MPT_ENUM(TypeIteratorPtr)) {
		if (ptr) *((const void **) ptr) = &m->_it;
		return MPT_ENUM(TypeArray);
	}
	if (type == 's') {
		if (ptr) {
			MPT_STRUCT(buffer) *buf = m->s._a._buf;
			const void *cmd = (buf && m->s._off) ? buf + 1 : 0;
			*((const void **) ptr) = cmd;
		}
		return MPT_ENUM(TypeIteratorPtr);
	}
	return MPT_ERROR(BadType);
}
static MPT_INTERFACE(metatype) *bufferCloneArgs(const MPT_INTERFACE(metatype) *mt)
{
	const MPT_STRUCT(metaBuffer) *m = (void *) mt;
	return bufferCopy(mpt_meta_arguments, &m->s);
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
		{ bufferConvArgs },
		bufferUnref,
		bufferRef,
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
	
	m->entry._conv._vptr = 0;
	m->entry.match = 0;
	m->entry.converter = 0;
	MPT_value_set(&m->entry.val, 0, 0);
	
	m->s = s;
	if (a) {
		mpt_array_clone(&m->s._a, a);
		if (bufferResetArgs(&m->_it) < 0) {
			bufferUnref(&m->_mt);
			errno = EINVAL;
			return 0;
		}
	}
	
	return &m->_mt;
}
