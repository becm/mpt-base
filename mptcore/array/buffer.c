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
 * \param size	hint to set new atomic allocation size
 */
extern int _mpt_buffer_realloc_align(size_t size)
{
	if (psize) {
		return -3;
	}
	if (size < 8 || size > 4*1024*1024) {
		return -2;
	}
	return psize = MPT_align(size);
}
/*!
 * \ingroup mptArray
 * \brief delete buffer data
 * 
 * Buffer resize modifier only allowing freeing
 * 
 * \param b	buffer pointer
 * \param size	requested new buffer size
 * 
 * \return new buffer sattisfying size
 */
extern MPT_STRUCT(buffer) *_mpt_buffer_free(MPT_STRUCT(buffer) *b, size_t size)
{
	 /* no size change of file maps (explicit unset needed) */
	if (size) {
		errno = ENOTSUP; return 0;
	}
	if (b && !(b->shared--)) {
		free(b);
	}
	return 0;
}
/*!
 * \ingroup mptArray
 * \brief resize buffer data
 * 
 * Buffer malloc resize modifier.
 * 
 * \param b	buffer pointer
 * \param size	requested new buffer size
 * 
 * \return new buffer sattisfying size
 */
extern MPT_STRUCT(buffer) *_mpt_buffer_realloc(MPT_STRUCT(buffer) *buf, size_t len)
{
	size_t	used, size;
	
	if (buf) {
		size = buf->size;
		used = buf->used;
	} else {
		size = used = 0;
	}
	if (!len) {
		if (buf && !(buf->shared--)) {
			free(buf);
		}
		return 0;
	}
	/* no change */
	if (len == size && (!buf || !buf->shared)) {
		return buf;
	}
	/* get buffer increase size (<page table) */
	if (!psize && _mpt_buffer_realloc_align(_MPT_BUFFER_PSTD) < 0) {
		return 0;
	}
	/* align to psize */
	len += sizeof(*buf);
	len = ((len - 1)/psize + 1) * psize;
	
	/* try to get in-place memory */
	if (buf && (len < size || len <= 1024 || used > size/2)) {
		if (!(buf = realloc(buf, len))) {
			return 0;
		}
		buf->size = len - sizeof(*buf);
		return buf;
	}
	/* try to get new memory in favour of less copying */
	else {
		MPT_STRUCT(buffer) *nb;
		
		if (!(nb = malloc(len))) {
			return 0;
		}
		nb->resize = _mpt_buffer_realloc;
		nb->shared = 0;
		nb->size   = len - sizeof(*nb);
		
		if ((nb->used = used)) {
			(void) memcpy(nb+1, buf+1, used);
		}
		if (buf && !(buf->shared--)) {
			free(buf);
		}
		return nb;
	}
}
