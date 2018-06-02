/*!
 * MPT core library
 *   array slice access
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
	const MPT_STRUCT(type_traits) *info;
	MPT_STRUCT(buffer) *buf;
	size_t total, used, size;
	uint8_t *ptr;
	
	/* invalid argument combination */
	if (len > (SIZE_MAX - off)) {
		errno = EINVAL;
		return 0;
	}
	total = off + len;
	
	/* new raw buffer */
	if (!(buf = arr->_buf)) {
		if (!(buf = _mpt_buffer_alloc(total, 0))) {
			return 0;
		}
		buf->_used = total;
		arr->_buf = buf;
		ptr = (uint8_t *) (buf + 1);
		if (off) {
			memset(buf + 1, 0, off);
			ptr += off;
		}
		return ptr;
	}
	used = buf->_used;
	size = total < used ? used : total;
	
	/* enforce data alignment */
	if ((info = buf->_typeinfo)) {
		size_t elem;
		if (!(elem = info->size)
		    || off % elem
		    || len % elem
		    || size % elem) {
			return 0;
		}
	}
	/* sufficient private space */
	if (total < buf->_size
	    && !buf->_vptr->shared(buf)) {
		ptr = (void *) (buf + 1);
		if (total <= used) {
			return ptr + off;
		}
		if (!(mpt_buffer_insert(buf, used, total - used))) {
			return 0;
		}
		if (info && !info->init) {
			memset(ptr + used, 0, total - used);
		}
		return ptr + off;
	}
	/* make private copy */
	if (!(buf = buf->_vptr->detach(buf, size))) {
		return 0;
	}
	arr->_buf = buf;
	
	return mpt_buffer_insert(buf, off, len);
}
