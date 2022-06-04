/*!
 * MPT core library
 *   copy buffer elements
 */

#include <string.h>

#include "types.h"

#include "array.h"

/*!
 * \ingroup mptArray
 * \brief copy buffer content
 * 
 * Replace target buffer content with data from source buffer elements.
 * If element copy is unsuccessful, default element value is used.
 * 
 * \param dest  data array
 * \param src   element in array
 * 
 * \return start address of array element
 */
extern long mpt_buffer_set(MPT_STRUCT(buffer) *buf, const MPT_STRUCT(type_traits) *src_traits, size_t pos, const void *src_data, size_t len)
{
	const MPT_STRUCT(type_traits) *traits;
	int  (*init)(void *, const void *);
	void (*fini)(void *);
	uint8_t *ptr;
	size_t end, used;
	size_t elem_size;
	
	if ((SIZE_MAX - pos) < len) {
		return MPT_ERROR(BadArgument);
	}
	if ((end = pos + len) > buf->_size) {
		return MPT_ERROR(MissingBuffer);
	}
	ptr = (uint8_t *) (buf + 1);
	
	/* copy/reset raw data */
	if (!(traits = buf->_content_traits)) {
		if (src_traits) {
			return MPT_ERROR(BadArgument);
		}
		if (src_data) {
			memcpy(ptr + pos, src_data, len);
		} else {
			memset(ptr + pos, 0, len);
		}
		if (end > buf->_used) {
			buf->_used = end;
		}
		return 0;
	}
	/* target area must align with element offsets */
	used = buf->_used;
	elem_size = 1;
	/* initialize with new type, remove old (in case of type replace) */
	init = 0;
	fini = traits->fini;
	if (src_traits) {
		elem_size = src_traits->size;
		if (!elem_size
		 || (pos % elem_size)
		 || (len % elem_size)) {
			return MPT_ERROR(BadArgument);
		}
		/* align uninitialized data position */
		used -= used % elem_size;
		init = src_traits->init;
	}
	if (traits != src_traits) {
		/* compatible types must share finalizer and size */
		if (!fini || !src_traits || (fini != src_traits->fini) || (traits->size != elem_size)) {
			return MPT_ERROR(BadType);
		}
	}
	/* terminate overlapping target data */
	if (fini) {
		size_t off;
		for (off = pos; off < used; off += elem_size) {
			fini(ptr + off);
		}
	}
	/* initialize prepending data */
	if (init) {
		size_t off;
		for (off = used; off < pos; off += elem_size) {
			if (init(ptr + off, 0) < 0) {
				buf->_used = off;
				return MPT_ERROR(BadOperation);
			}
		}
	}
	else if (used < pos) {
		memset(ptr + used, 0, pos - used);
	}
	/* generic data copy */
	if (!init) {
		if (src_data) {
			memcpy(ptr + pos, src_data, len);
		} else {
			memset(ptr + pos, 0, len);
		}
		buf->_used = (used < end) ? end : used;
		return len / elem_size;
	}
	/* prepare target and copy data */
	else {
		const uint8_t *from = src_data;
		long count = 0;
		
		/* initialize target values from source */
		while (pos < end) {
			/* try regular copy-init */
			if (src_data && init(ptr + pos, from) >= 0) {
				count++;
			}
			/* fall back to generic init */
			else if (init(ptr + pos, 0) < 0) {
				/* invalidate remaining data as result of fatal error */
				buf->_used = pos;
				if (fini) {
					while (pos < used) {
						fini(ptr + pos);
						pos += elem_size;
					}
				}
				return count;
			}
			pos += elem_size;
			from += elem_size;
		}
		/* update target size */
		buf->_used = (used < end) ? end : used;
		
		return count;
	}
}
