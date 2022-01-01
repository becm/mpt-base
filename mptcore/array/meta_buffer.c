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
	
	MPT_STRUCT(slice) s;
};

/* element convertable interface */
static int bufferConvertEntry(MPT_INTERFACE(convertable) *conv, int type, void *ptr)
{
	MPT_STRUCT(metaBuffer) *m = MPT_baseaddr(metaBuffer, conv, entry._conv);
	const MPT_STRUCT(buffer) *buf;
	const MPT_STRUCT(type_traits) *traits;
	
	if (!(buf = m->s._a._buf)) {
		return MPT_ERROR(MissingData);
	}
	if (!(traits = buf->_content_traits)) {
		return MPT_ERROR(BadType);
	}
	if (buf != m->entry.match) {
		return MPT_ERROR(BadOperation);
	}
	if (m->entry.converter) {
		const uint8_t *data = (void *) (buf + 1);
		return m->entry.converter(data + m->s._off, type, ptr);
	}
	return mpt_slice_get(&m->s, type, ptr);
}
/* iterator interface */
static const MPT_STRUCT(value) *bufferGet(MPT_INTERFACE(iterator) *it)
{
	MPT_STRUCT(metaBuffer) *m = MPT_baseaddr(metaBuffer, it, _it);
	return &m->entry.val;
}
static int bufferAdvance(MPT_INTERFACE(iterator) *it)
{
	MPT_STRUCT(metaBuffer) *m = MPT_baseaddr(metaBuffer, it, _it);
	const MPT_STRUCT(buffer) *buf = m->s._a._buf;
	const MPT_STRUCT(type_traits) *traits;
	
	if (!buf) {
		return MPT_ERROR(MissingData);
	}
	if (buf != m->entry.match) {
		return MPT_ERROR(BadOperation);
	}
	if (!(traits = buf->_content_traits)) {
		return MPT_ERROR(BadOperation);
	}
	if (traits != mpt_type_traits(MPT_type_toVector('c'))) {
		if (!traits->size) {
			return MPT_ERROR(BadValue);
		}
		m->s._off += m->s._len;
		m->s._len = traits->size;
		return buf->_used > m->s._off;
	}
	if (!m->s._len) {
		int type = mpt_slice_get(&m->s, MPT_ENUM(TypeVector), 0);
		if (type < 0) {
			return type;
		}
		if (!m->s._len) {
			return 0;
		}
	}
	m->s._off += m->s._len;
	m->s._len = 0;
	return buf->_used > m->s._off ? 's' : 0;
}
static int bufferReset(MPT_INTERFACE(iterator) *it)
{
	MPT_STRUCT(metaBuffer) *m = MPT_baseaddr(metaBuffer, it, _it);
	const MPT_STRUCT(buffer) *buf = m->s._a._buf;
	const MPT_STRUCT(type_traits) *traits = buf ? buf->_content_traits : 0;
	
	m->entry.match = buf;
	
	m->s._off = 0;
	m->s._len = buf ? buf->_used : 0;
	return traits && (traits == mpt_type_traits('c')) ? 1 : 0;
}
/* convertable interface */
static int bufferConv(MPT_INTERFACE(convertable) *val, int type, void *ptr)
{
	const MPT_STRUCT(metaBuffer) *m = (void *) val;
	
	if (!type) {
		static const uint8_t fmt[] = { MPT_ENUM(TypeIteratorPtr), MPT_type_toVector('c'), 0 };
		if (ptr) {
			*((const uint8_t **) ptr) = fmt;
			return MPT_ENUM(TypeArray);
		}
		return MPT_ENUM(TypeIteratorPtr);
	}
	if (type == MPT_ENUM(TypeIteratorPtr)) {
		if (ptr) *((const void **) ptr) = &m->_it;
		return MPT_ENUM(TypeArray);
	}
	if (type == MPT_type_toVector('c')
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
	MPT_INTERFACE(convertable) *conv;
	
	if (!(m = malloc(sizeof(*m)))) {
		return 0;
	}
	m->_mt._vptr = &metaBuffer;
	m->_it._vptr = &iterBuffer;
	
	conv = &m->entry._conv;
	m->entry._conv._vptr = &convBufferEntry;
	*((uint8_t *) &m->entry.val._bufsize) = sizeof(m->entry.val._buf);
	MPT_value_set_data(&m->entry.val, MPT_ENUM(TypeConvertablePtr), &conv);
	
	m->s = s;
	if (a) {
		mpt_array_clone(&m->s._a, a);
	}
	m->entry.match = m->s._a._buf;
	return &m->_mt;
}
/* return first argument as string content */
static const MPT_STRUCT(value) *bufferValue(MPT_INTERFACE(iterator) *it)
{
	MPT_STRUCT(metaBuffer) *m = MPT_baseaddr(metaBuffer, it, _it);
	struct iovec vec;
	int ret;
	if ((ret = mpt_slice_get(&m->s, MPT_type_toVector('c'), &vec)) < 0) {
		return 0;
	}
	vec.iov_base = ((char *) (m->s._a._buf + 1)) + m->s._off;
	vec.iov_len  = m->s._len;
	
	MPT_value_set_data(&m->entry.val, MPT_type_toVector('c'), &vec);
	
	return &m->entry.val;
}
/* return first argument as string content */
static int bufferResetArgs(MPT_INTERFACE(iterator) *it)
{
	MPT_STRUCT(metaBuffer) *m = MPT_baseaddr(metaBuffer, it, _it);
	struct iovec vec;
	int ret;
	if ((ret = bufferReset(it)) <= 0) {
		return ret;
	}
	if ((ret = mpt_slice_get(&m->s, MPT_ENUM(TypeVector), &vec)) < 0) {
		return ret;
	}
	m->s._off += m->s._len;
	m->s._len = 0;
	return (m->s._a._buf->_used > m->s._off) ? 1 : 0;
}
static int bufferConvArgs(MPT_INTERFACE(convertable) *val, int type, void *ptr)
{
	const MPT_STRUCT(metaBuffer) *m = (void *) val;
	
	if (!type) {
		static const uint8_t fmt[] = { MPT_ENUM(TypeIteratorPtr), 's', 0 };
		if (ptr) {
			*((const uint8_t **) ptr) = fmt;
			return MPT_ENUM(TypeArray);
		}
		return MPT_ENUM(TypeMetaPtr);
	}
	if (type == 's') {
		if (ptr) {
			MPT_STRUCT(buffer) *buf = m->s._a._buf;
			if (buf && m->s._off) {
				++buf;
			}
			*((const void **) ptr) = buf;
		}
		return MPT_ENUM(TypeIteratorPtr);
	}
	if (type == MPT_ENUM(TypeIteratorPtr)) {
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
		bufferValue,
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
	m->entry.val.type = 0;
	
	m->s = s;
	if (a) {
		mpt_array_clone(&m->s._a, a);
		if (bufferResetArgs(&m->_it) < 0) {
			bufferUnref(&m->_mt);
			errno = EINVAL;
			return 0;
		}
		m->s._off += m->s._len;
		m->s._len = 0;
	}
	
	return &m->_mt;
}
