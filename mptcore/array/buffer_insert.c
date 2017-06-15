/*!
 * insert data in array
 */

#include <errno.h>
#include <string.h>

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
extern void *mpt_buffer_insert(MPT_STRUCT(buffer) *buf, size_t pos, size_t len)
{
	uint8_t *base;
	size_t used, min, total;
	
	if (len > (SIZE_MAX - pos)) {
		errno = EINVAL;
		return 0;
	}
	total = len + pos;
	if (!total) {
		return buf + 1;
	}
	if (!buf) {
		errno = EINVAL;
		return 0;
	}
	used = buf->_used;
	if (len > (SIZE_MAX - used)) {
		errno = EINVAL;
		return 0;
	}
	min = pos > used ? pos : used;
	
	/* need memory for inserted and existing data */
	if (len > (SIZE_MAX - min)
	    || buf->_size < (min + len)) {
		errno = EINVAL;
		return 0;
	}
	base = ((uint8_t *) (buf + 1)) + pos;
	if (pos > used) {
		/* fill preceeding data */
		pos -= used;
		memset(base - pos, 0, pos);
	}
	else if ((pos = used - pos)) {
		/* move data after position */
		memmove(base + len, base, pos);
	}
	buf->_used = min + len;
	return base;
}
