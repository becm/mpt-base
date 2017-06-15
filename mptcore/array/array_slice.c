/*!
 * array slice access
 */

#include <errno.h>
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
	size_t total, used, max;
	uint8_t *ptr;
	
	if (len > (SIZE_MAX - off)) {
		errno = EINVAL;
		return 0;
	}
	total = off + len;
	
	/* require raw buffer data */
	if ((buf = arr->_buf)
	  && buf->_vptr->content(buf)) {
		buf->_vptr->ref.unref((void *) buf);
		buf = 0;
	}
	if (!buf) {
		if (!len || !(buf = _mpt_buffer_alloc(total))) {
			return 0;
		}
		buf->_used = total;
		arr->_buf = buf;
		if (off) memset(buf + 1, 0, off);
		return ((uint8_t *) (buf + 1)) + off;
	}
	used = buf->_used;
	
	/* no shared data */
	if (buf->_ref._val < 2) {
		ptr = ((uint8_t *) (buf + 1)) + off;
		/* slice in existing data */
		if (total <= used) {
			return ptr;
		}
		if (total < buf->_size) {
			if (off > used) {
				memset(ptr, 0, used - off);
			}
			buf->_used = total;
			return ptr;
		}
	}
	max = total < used ? used : total;
	if (!(buf = buf->_vptr->detach(buf, max))) {
		return 0;
	}
	ptr = ((uint8_t *) (buf + 1)) + off;
	if (off > used) {
		memset(ptr, 0, used - off);
	}
	if (used < total) {
		buf->_used = total;
	}
	return buf;
}
