/*!
 * MPT core library
 *   control and initializer for heap-based data.
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "array.h"

#ifndef _MPT_BUFFER_PSTD
# define _MPT_BUFFER_PSTD 128
#endif

static int _mpt_buffer_alloc_psize = 0;

/*!
 * \ingroup mptArray
 * \brief set atomic buffer size
 * 
 * Set atomic part size for buffer.
 * 
 * \param size  hint to set new atomic allocation size
 */
static int _mpt_buffer_alloc_align(size_t size)
{
	if (!size) {
		return _mpt_buffer_alloc_psize = _MPT_BUFFER_PSTD;
	}
	if (size < 8 || size > 4*1024*1024) {
		return MPT_ERROR(BadValue);
	}
	return _mpt_buffer_alloc_psize = MPT_align(size);
}

MPT_STRUCT(bufferData)
{
	MPT_STRUCT(refcount) _ref;
	MPT_STRUCT(type_traits) *info;
	size_t _psize;
	
	/* align memory boundary */
	uint8_t _pad[8 * sizeof(void *)
	           - sizeof(MPT_STRUCT(refcount))
	           - sizeof(MPT_STRUCT(type_traits) *)
	           - sizeof(size_t)
	           - sizeof(MPT_STRUCT(buffer))
	];
	
	MPT_STRUCT(buffer) buf;
};
/* reference interface */
static void _mpt_buffer_alloc_unref(MPT_INTERFACE(instance) *in)
{
	MPT_STRUCT(bufferData) *buf = MPT_baseaddr(bufferData, in, buf);
	const MPT_STRUCT(type_traits) *info;
	if (mpt_refcount_lower(&buf->_ref)) {
		return;
	}
	if ((info = buf->buf._typeinfo)) {
		void (*fini)(void *);
		size_t size;
		if ((size = info->size)
		    && (fini = info->fini)) {
			size_t i, len = buf->buf._used;
			uint8_t *ptr = (void *) (buf + 1);
			len -= len % size;
			for (i = 0; i < len; i += size) {
				fini(ptr + i);
			}
		}
	}
	if (buf->info) {
		free(buf->info);
	}
	free(buf);
}
static uintptr_t _mpt_buffer_alloc_ref(MPT_INTERFACE(instance) *in)
{
	MPT_STRUCT(bufferData) *buf = MPT_baseaddr(bufferData, in, buf);
	return mpt_refcount_raise(&buf->_ref);
}
/* buffer interface */
static int _mpt_buffer_alloc_shared(const MPT_STRUCT(buffer) *ptr)
{
	MPT_STRUCT(bufferData) *buf = MPT_baseaddr(bufferData, ptr, buf);
	return buf->_ref._val > 1 ? 1 : 0;
}
static MPT_STRUCT(buffer) *_mpt_buffer_alloc_detach(MPT_STRUCT(buffer) *, size_t);
static const MPT_INTERFACE_VPTR(buffer) _mpt_buffer_vptr = {
	{ _mpt_buffer_alloc_unref, _mpt_buffer_alloc_ref },
	_mpt_buffer_alloc_detach,
	_mpt_buffer_alloc_shared
};
static MPT_STRUCT(buffer) *_mpt_buffer_alloc_detach(MPT_STRUCT(buffer) *ptr, size_t len)
{
	MPT_STRUCT(bufferData) *next, *buf = MPT_baseaddr(bufferData, ptr, buf);
	const MPT_STRUCT(type_traits) *info;
	size_t add;
	
	/* get real required size */
	if ((info = buf->buf._typeinfo)) {
		size_t size;
		if (!(size = info->size)) {
			return 0;
		}
		/* align size info */
		add = len % size;
		if (add) {
			len += size - add;
		}
	}
	/* no modification required */
	if (buf->_ref._val < 2) {
		if (len <= buf->buf._size) {
			return &buf->buf;
		}
	}
	/* require valid copy operation */
	else if (info
	         && info->init
	         && !info->copy) {
		return 0;
	}
	/* add management info size */
	len += sizeof(*buf);
	
	/* align to creation time part size */
	add = len % buf->_psize;
	if (add) {
		len += buf->_psize - add;
	}
	if (!(next = malloc(len))) {
		return 0;
	}
	/* copy type information */
	if (info) {
		void *ptr;
		if (!(ptr = malloc(sizeof(*info)))) {
			free(next);
			return 0;
		}
		next->info = memcpy(ptr, info, sizeof(*info));
	} else {
		next->info = 0;
	}
	next->_psize = buf->_psize;
	next->_ref._val = 1;
	
	len -= sizeof(*next);
	next->buf._vptr = &_mpt_buffer_vptr;
	next->buf._typeinfo = next->info;
	next->buf._size = len;
	next->buf._used = 0;
	
	/* require content copy */
	if (mpt_refcount_lower(&buf->_ref)) {
		if (mpt_buffer_copy(&next->buf, &buf->buf) < 0) {
			if (next->info) {
				free(next->info);
			}
			free(next);
			return 0;
		}
	}
	/* move data content */
	else if ((add = buf->buf._used)) {
		if (add > len) {
			void (*fini)(void *);
			if (!info) {
				add = len;
			}
			else if ((fini = info->fini)) {
				size_t pos, size = info->size;
				uint8_t *ptr = (void *) (buf + 1);
				add -= (add % size);
				len -= (len % size);
				
				for (pos = len; pos < add; pos += size) {
					fini(ptr + pos);
				}
				add = len;
			}
		}
		memcpy(next + 1, buf + 1, add);
		next->buf._used = add;
		if (buf->info) {
			free(buf->info);
		}
		free(buf);
	}
	return &next->buf;
}
/*!
 * \ingroup mptArray
 * \brief buffer data
 * 
 * Raw data buffer with malloc resize modifier.
 * 
 * \param size  requested new buffer size
 * 
 * \return new buffer sattisfying (at least) size
 */
extern MPT_STRUCT(buffer) *_mpt_buffer_alloc(size_t len, const MPT_STRUCT(type_traits) *info)
{
	MPT_STRUCT(bufferData) *b;
	
	/* get buffer increase size (<page table) */
	if (!_mpt_buffer_alloc_psize
	    && _mpt_buffer_alloc_align(0) < 0) {
		return 0;
	}
	/* persistent type information */
	len += sizeof(*b);
	len = (((len - 1) / _mpt_buffer_alloc_psize) + 1) * _mpt_buffer_alloc_psize;
	if (!(b = malloc(len))) {
		return 0;
	}
	b->_ref._val = 1;
	if (!info) {
		b->info = 0;
	} else {
		void *ptr;
		if (!(ptr = malloc(sizeof(*info)))) {
			free(b);
			return 0;
		}
		info = memcpy(ptr, info, sizeof(*info));
		b->info = ptr;
	}
	b->_psize = _mpt_buffer_alloc_psize;
	
	b->buf._vptr = &_mpt_buffer_vptr;
	b->buf._typeinfo = info;
	b->buf._size = len - sizeof(*b);
	b->buf._used = 0;
	
	return &b->buf;
}
