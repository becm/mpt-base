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
	/* reject raw data */
	if (!src_traits) {
		return MPT_ERROR(BadArgument);
	}
	/* sizes for source and target data must match */
	elem_size = traits->size;
	if (!elem_size || (elem_size != src_traits->size)) {
		return MPT_ERROR(BadType);
	}
	/* target area must align with element offsets */
	if ((pos % elem_size)
	    || (len % elem_size)) {
		return MPT_ERROR(BadArgument);
	}
	
	init = traits->init;
	fini = traits->fini;
	/* trivial types must match exactly */
	if (!init) {
		if (traits != src_traits) {
			return MPT_ERROR(BadType);
		}
	}
	/* compatible types must share initializer */
	else if (init != src_traits->init) {
		return MPT_ERROR(BadType);
	}
	used = buf->_used;
	if (!src_data) {
		/* block termination of non-trivial content */
		if (init || fini) {
			return MPT_ERROR(BadArgument);
		}
		if (used < pos) {
			len += pos - used;
			pos = used;
		}
		buf->_used = pos + len;
		memset(ptr + pos, 0, len);
		return len / elem_size;
	}
	/* initialize prepending data */
	if (init) {
		for (; used < pos; used += elem_size) {
			if (init(ptr + used, 0) < 0) {
				buf->_used = used;
				return MPT_ERROR(BadOperation);
			}
		}
	}
	else if (used < pos) {
		memset(ptr + used, 0, pos - used);
	}
	/* terminate required target data */
	if (fini && (pos < used)) {
		size_t off, clear_end = used - pos;
		if (clear_end >= len) {
			clear_end = end;
		}
		/* end position exceeds existing data */
		else {
			clear_end += pos - (clear_end % elem_size);
		}
		for (off = pos; off < clear_end; off += elem_size) {
			fini(ptr + off);
		}
	}
	
	/* generic data copy */
	if (!init) {
		memcpy(ptr + pos, src_data, len);
		if (buf->_used < end) {
			buf->_used = end;
		}
		return len / elem_size;
	}
	/* prepare target and copy data */
	else {
		const uint8_t *from = src_data;
		long count = 0;
		
		/* initialize target values from source */
		while (pos < end) {
			/* try regular copy-init */
			if (init(ptr + pos, from) >= 0) {
				count++;
			}
			/* fall back to generic init */
			else if (init(ptr + pos, 0) < 0) {
				size_t used = buf->_used;
				/* invalidate remaining data as result of fatal error */
				buf->_used = pos;
				if (fini) {
					used -= used % elem_size;
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
		/* grow target size */
		if (buf->_used < end) {
			buf->_used = end;
		}
		return count;
	}
}
