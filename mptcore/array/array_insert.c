/*!
 * insert data in array
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
	/* need raw buffer data */
	if (!(b = arr->_buf)
	  && (b->_vptr->content(b))) {
		b->_vptr->ref.unref((void *) b);
		b = 0;
	}
	/* create new raw buffer */
	if (!b) {
		uint8_t *base;
		len += pos;
		if (!(b = _mpt_buffer_alloc(len))) {
			return 0;
		}
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
	if (!(b = b->_vptr->detach(b, used + len))) {
		return 0;
	}
	return mpt_buffer_insert(b, pos, len);
}
