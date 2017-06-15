/*!
 * control and initializer for heap-based data.
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

static int psize = 0;

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
	if (psize) {
		return -3;
	}
	if (size < 8 || size > 4*1024*1024) {
		return -2;
	}
	return psize = MPT_align(size);
}
static void _mpt_buffer_alloc_unref(MPT_INTERFACE(unrefable) *ref)
{
	MPT_STRUCT(buffer) *buf = (void *) ref;
	
	if (!mpt_reference_lower(&buf->_ref)) {
		free(buf);
	}
}
static const MPT_INTERFACE_VPTR(buffer) _mpt_buffer_vptr;
static MPT_STRUCT(buffer) *_mpt_buffer_alloc_detach(MPT_STRUCT(buffer) *buf, long len)
{
	MPT_STRUCT(buffer) *b;
	long used = buf->_used;
	
	if (len < 0) {
		len = used;
	}
	if (buf->_ref._val > 1) {
		if (!(b = _mpt_buffer_alloc(len))) {
			return 0;
		}
	}
	else {
		long size = buf->_size, align;
		
		if (!len) {
			len = psize;
		}
		else if ((align = len % psize)) {
			len += psize - align;
		}
		/* try to get in-place memory */
		if (len < size || len <= 1024 || used > size/4) {
			if (!(b = realloc(buf, len))) {
				return 0;
			}
			len -= sizeof(*b);
			b->_size = len;
			if (len < used) {
				buf->_used = len;
			}
			return b;
		}
		/* get new memory in favour of less copying */
		else if (!(b = _mpt_buffer_alloc(len))) {
			return 0;
		}
	}
	/* copy old data */
	if ((b->_used = used)) {
		(void) memcpy(b + 1, buf + 1, used);
	}
	/* clear data reference */
	if (!mpt_reference_lower(&buf->_ref)) {
		free(buf);
	}
	return b;
}
static int _mpt_buffer_alloc_type(const MPT_STRUCT(buffer) *buf)
{
	(void) buf;
	return 0;
}
static const MPT_INTERFACE_VPTR(buffer) _mpt_buffer_vptr = {
	{ _mpt_buffer_alloc_unref },
	_mpt_buffer_alloc_detach,
	_mpt_buffer_alloc_type
};
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
extern MPT_STRUCT(buffer) *_mpt_buffer_alloc(size_t len)
{
	MPT_STRUCT(buffer) *b;
	
	/* get buffer increase size (<page table) */
	if (!psize && _mpt_buffer_alloc_align(_MPT_BUFFER_PSTD) < 0) {
		return 0;
	}
	/* align to psize */
	len += sizeof(*b);
	len = ((len - 1) / psize + 1) * psize;
	
	if (!(b = malloc(len))) {
		return 0;
	}
	b->_vptr = &_mpt_buffer_vptr;
	b->_ref._val = 1;
	b->_size = len - sizeof(*b);
	b->_used = 0;
	
	return b;
}
