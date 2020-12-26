/*!
 * insert data in array
 */

#include <errno.h>
#include <string.h>

#include "types.h"

#include "array.h"

/*!
 * \ingroup mptArray
 * \brief insert raw buffer data
 * 
 * Insert data in buffer on position.
 * Need sufficient size for operation.
 * 
 * The inserted region is NOT initialized!
 * 
 * \param buf  buffer data
 * \param pos  where to insert
 * \param len  inserted data length
 * 
 * \return start address added raw data
 */
extern void *mpt_buffer_insert(MPT_STRUCT(buffer) *buf, size_t pos, size_t len)
{
	const MPT_STRUCT(type_traits) *traits;
	int (*init)(void *, const void *);
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
	size = 0;
	init = 0;
	if ((traits = buf->_content_traits)) {
		if (!(size = traits->size)
		    || used % size
		    || pos % size
		    || len % size) {
			errno = EINVAL;
			return 0;
		}
		init = traits->init;
	}
	base = (uint8_t *) (buf + 1);
	buf->_used = total;
	
	/* move data after insert position */
	if (keep) {
		memmove(base + total - keep, base + pos, keep);
	}
	/* init all new data */
	if (init) {
		while (used < pos) {
			if (init(base + used, 0) < 0) {
				break;
			}
			used += size;
		}
	}
	/* fill preceeding data */
	if (pos > used) {
		memset(base + used, 0, pos - used);
	}
	return base + pos;
}
