/*!
 * array slice access
 */

#include <string.h>

#include "array.h"
/*!
 * \ingroup mptArray
 * \brief get array slice
 * 
 * Get data part of array.
 * Extend and zero data on array if needed.
 * 
 * \param arr  array descriptor
 * \param off  offset for requested data
 * \param len  length of requested data
 * 
 * \return start address of data
 */
extern void *mpt_array_slice(MPT_STRUCT(array) *arr, size_t off, size_t len)
{
	MPT_STRUCT(buffer) *buf;
	size_t	total = off + len, used;
	
	if (!(buf = arr->_buf)) {
		if (!len) return 0;
		if (!(buf = _mpt_buffer_realloc(0, total))) return 0;
		buf->used = total;
		arr->_buf = buf;
		if (off) memset(buf+1, 0, off);
		return ((uint8_t *) (buf+1)) + off;
	}
	used = buf->used;
	
	/* no shared data */
	if (!buf->shared) {
		/* slice in existing data */
		if (total <= used)
			return ((uint8_t *) (buf+1)) + off;
		
		/* resize needed */
		if (total > buf->size) {
			if (!buf->resize || !(buf = buf->resize(buf, total)))
				return 0;
			arr->_buf = buf;
		}
	}
	/* buffer data is shared */
	else {
		if (used > total) total = used;
		
		/* forced resize of shared data */
		if (!buf->resize || !(buf = buf->resize(buf, total)))
			return 0;
		
		arr->_buf = buf;
	}
	buf->used = total;
	
	/* initialisation before offset */
	if (used < off) {
		memset(((uint8_t *) (buf+1))+used, 0, off-used);
	}
	
	return ((uint8_t *) (buf+1)) + off;
}
