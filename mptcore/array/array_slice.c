/*!
 * MPT core library
 *   array slice access
 */

#include <errno.h>
#include <string.h>

#include "types.h"

#include "array.h"
/*!
 * \ingroup mptArray
 * \brief get array slice
 * 
 * Get data part of array.
 * Extend and initialize data on array if needed.
 * 
 * \param arr  array descriptor
 * \param off  offset for requested data
 * \param len  length of requested data
 * 
 * \return start address of data
 */
extern void *mpt_array_slice(MPT_STRUCT(array) *arr, size_t off, size_t len)
{
	const MPT_STRUCT(type_traits) *traits;
	MPT_STRUCT(buffer) *buf;
	size_t total, used, size;
	int flags;
	
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
		if (total) {
			memset(buf + 1, 0, total);
		}
		return ((uint8_t *) (buf + 1)) + off;
	}
	used = buf->_used;
	size = total < used ? used : total;
	
	/* enforce data alignment */
	if ((traits = buf->_content_traits)) {
		size_t elem;
		if (!(elem = traits->size)
		    || off % elem
		    || len % elem
		    || used % elem) {
			return 0;
		}
	}
	
	flags = buf->_vptr->get_flags(buf);
	/* require sufficent private space */
	if (total > buf->_size
	 || (MPT_ENUM(BufferImmutable) & flags)
	 || (MPT_ENUM(BufferShared) & flags)) {
		/* make private copy */
		if (!(buf = buf->_vptr->detach(buf, size))) {
			return 0;
		}
		arr->_buf = buf;
	}
	/* insufficient data on buffer */
	if (total > used) {
		int (*init)(void *, const void *);
		size_t missing = total - used;
		uint8_t *ptr;
		if (!(ptr = mpt_buffer_insert(buf, used, missing))) {
			return 0;
		}
		if (traits && (init = traits->init)) {
			size_t pos, adv = traits->size;
			for (pos = 0; pos < missing; pos += adv) {
				init(ptr + pos, 0);
			}
		}
		else {
			memset(ptr, 0, missing);
		}
	}
	return ((uint8_t *) (buf + 1)) + off;
}
