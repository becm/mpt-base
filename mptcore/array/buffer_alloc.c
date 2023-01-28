/*!
 * MPT core library
 *   control and initializer for heap-based data.
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "types.h"

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
	size_t _psize;
	int _flags;
	
	/* align memory boundary */
	uint8_t _pad[8 * sizeof(void *)
	           - sizeof(MPT_STRUCT(refcount))
	           - sizeof(size_t)
	           - sizeof(int)
	           - sizeof(MPT_STRUCT(buffer))
	];
	
	MPT_STRUCT(buffer) buf;
};

/* reference interface */
static void _mpt_buffer_alloc_unref(MPT_INTERFACE(buffer) *val)
{
	MPT_STRUCT(bufferData) *buf = MPT_baseaddr(bufferData, val, buf);
	const MPT_STRUCT(type_traits) *traits;
	if (mpt_refcount_lower(&buf->_ref)) {
		return;
	}
	if ((traits = buf->buf._content_traits)) {
		void (*fini)(void *) = traits->fini;
		size_t size = traits->size;
		if (size && fini) {
			size_t pos, len = buf->buf._used;
			uint8_t *ptr = (void *) (buf + 1);
			len -= len % size;
			for (pos = 0; pos < len; pos += size) {
				fini(ptr + pos);
			}
		}
	}
	free(buf);
}
static uintptr_t _mpt_buffer_alloc_ref(MPT_INTERFACE(buffer) *val)
{
	MPT_STRUCT(bufferData) *buf = MPT_baseaddr(bufferData, val, buf);
	return mpt_refcount_raise(&buf->_ref);
}
/* buffer interface */
static uint32_t _mpt_buffer_alloc_flags(const MPT_STRUCT(buffer) *ptr)
{
	MPT_STRUCT(bufferData) *buf = MPT_baseaddr(bufferData, ptr, buf);
	int flags = buf->_ref._val > 1 ? MPT_ENUM(BufferShared) : 0;
	return flags | buf->_flags;
}
static MPT_STRUCT(buffer) *_mpt_buffer_alloc_detach(MPT_STRUCT(buffer) *ptr, size_t len)
{
	MPT_STRUCT(bufferData) *buf = MPT_baseaddr(bufferData, ptr, buf);
	MPT_STRUCT(buffer) *next;
	const MPT_STRUCT(type_traits) *traits;
	
	/* get real required size */
	if ((traits = buf->buf._content_traits)) {
		size_t size, add;
		if (!(size = traits->size)) {
			return 0;
		}
		/* align size */
		add = len % size;
		if (add) {
			len += size - add;
		}
	}
	/* no modification required */
	if (buf->_ref._val < 2) {
		if (len <= buf->buf._size
		 && !(buf->_flags & MPT_ENUM(BufferImmutable))) {
			return &buf->buf;
		}
	}
	/* block copy of data */
	else if ((buf->_flags & MPT_ENUM(BufferNoCopy)) && buf->buf._used) {
		errno = ENOTSUP;
		return 0;
	}
	/* clear immutability flag on clone */
	if (!(next = _mpt_buffer_alloc(len, buf->_flags & ~MPT_ENUM(BufferImmutable)))) {
		return 0;
	}
	next->_content_traits = traits;
	
	/* require content copy */
	if (mpt_refcount_lower(&buf->_ref)) {
		const MPT_STRUCT(buffer) *src = &buf->buf;
		if (mpt_buffer_set(next, src->_content_traits, 0, src + 1, src->_used) < 0) {
			_mpt_buffer_alloc_unref(next);
			return 0;
		}
	}
	/* move data content */
	else {
		size_t add = buf->buf._used;
		if (add > len) {
			void (*fini)(void *);
			if (!traits) {
				add = len;
			}
			else if ((fini = traits->fini)) {
				uint8_t *ptr = (void *) (buf + 1);
				size_t pos, esize = traits->size;
				/* align used data */
				add -= (add % esize);
				/* length already aligned in initial fixup */
				
				for (pos = len; pos < add; pos += esize) {
					fini(ptr + pos);
				}
				add = len;
			}
		}
		/* copy remaining data to new location */
		if (add) {
			memcpy(next + 1, buf + 1, add);
		}
		next->_used = add;
		/* remove old data reference */
		free(buf);
	}
	return next;
}

/*!
 * \ingroup mptArray
 * \brief buffer data
 * 
 * Raw data buffer with malloc resize modifier.
 * 
 * \param len    requested new buffer size
 * \param flags  static user flags
 * 
 * \return new buffer sattisfying (at least) size
 */
extern MPT_STRUCT(buffer) *_mpt_buffer_alloc(size_t len, int flags)
{
	static const MPT_INTERFACE_VPTR(buffer) _mpt_buffer_vptr = {
		_mpt_buffer_alloc_flags,
		_mpt_buffer_alloc_unref,
		_mpt_buffer_alloc_ref,
		_mpt_buffer_alloc_detach
	};
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
	b->_psize = _mpt_buffer_alloc_psize;
	b->_flags = flags & MPT_ENUM(BufferFlagsUser);
	
	b->buf._vptr = &_mpt_buffer_vptr;
	b->buf._content_traits = 0;
	*((size_t *) &b->buf._size) = len - sizeof(*b);
	b->buf._used = 0;
	
	return &b->buf;
}
