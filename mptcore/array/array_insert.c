/*!
 * MPT core library
 *   insert data in array
 */

#include <errno.h>
#include <string.h>
#include <limits.h>

#include "array.h"

/*!
 * \ingroup mptArray
 * \brief insert buffer data
 * 
 * Insert data in buffer on position.
 * Need sufficient size for operation.
 * 
 * \param buf  buffer data
 * \param pos  where to insert
 * \param len  inserted data length
 * 
 * \return start address added data
 */
extern void *mpt_array_insert(MPT_STRUCT(array) *arr, size_t pos, size_t len)
{
	MPT_STRUCT(buffer) *b;
	size_t used;
	
	if (len > (LONG_MAX - pos)) {
		errno = EINVAL;
		return 0;
	}
	/* create new raw buffer */
	if (!(b = arr->_buf)) {
		uint8_t *base;
		len += pos;
		if (!(b = _mpt_buffer_alloc(len, 0))) {
			return 0;
		}
		arr->_buf = b;
		base = (void *) (b + 1);
		if (pos) {
			memset(base, 0, pos);
		}
		b->_used = len;
		return base + pos;
	}
	used = b->_used;
	if (pos > used) {
		used = pos;
	}
	/* sufficient private space */
	if ((used + len) <= b->_size
	    && !(b->_vptr->get_flags(b) & MPT_ENUM(BufferShared))) {
		return mpt_buffer_insert(b, pos, len);
	}
	if (!(b = b->_vptr->detach(b, used + len))) {
		return 0;
	}
	arr->_buf = b;
	return mpt_buffer_insert(b, pos, len);
}
