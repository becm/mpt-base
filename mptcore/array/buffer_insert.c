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
	const MPT_STRUCT(type_traits) *info;
	void (*init)(const MPT_STRUCT(type_traits) *, void *);
	uint8_t *base;
	size_t used, total, size, keep;
	
	/* need memory for inserted and existing data */
	used = buf->_used;
	if (pos < used) {
		total = used + len;
		keep = used - pos;
	} else {
		total = pos + len;
		keep = 0;
	}
	if (!total) {
		return buf + 1;
	}
	if (total > buf->_size) {
		errno = EINVAL;
		return 0;
	}
	/* require aligned values */
	init = 0;
	size = 0;
	if ((info = buf->_typeinfo)) {
		init = info->init;
		if (!(size = info->size)
		    || used % size
		    || pos % size
		    || len % size) {
			errno = EINVAL;
			return 0;
		}
	}
	base = (uint8_t *) (buf + 1);
	buf->_used = total;
	
	/* move data after insert position */
	if (keep) {
		memmove(base + total - keep, base + pos, keep);
	}
	/* init all new data */
	if (init) {
		size_t curr, end;
		curr = used - keep;
		end = total - keep;
		while (curr < end) {
			init(info, base + curr);
			curr += size;
		}
	}
	/* fill preceeding data */
	else if (pos > used) {
		memset(base + used, 0, pos - used);
	}
	return base + pos;
}
