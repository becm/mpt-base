/*!
 * remove data from array
 */

#include <string.h>

#include "array.h"

/*!
 * \ingroup mptArray
 * \brief remove buffer data
 * 
 * Cut data from buffer starting from offset.
 * 
 * \param buf  data array
 * \param off  start position to remove
 * \param len  length of element to remove
 * 
 * \return size of moved data
 */
ssize_t mpt_buffer_cut(MPT_STRUCT(buffer) *buf, size_t off, size_t len)
{
	const MPT_STRUCT(type_traits) *info;
	uint8_t *pos;
	size_t keep;
	
	if (len > buf->_used) {
		return MPT_ERROR(BadArgument);
	}
	/* only keep data till offset */
	if (!len) {
		keep = off;
	}
	/* dat must be in range */
	else if ((keep = buf->_used - len) < off) {
		return MPT_ERROR(MissingData);
	}
	/* data start */
	pos = ((uint8_t *) (buf + 1)) + off;
	
	/* invalidate data to cut */
	if ((info = buf->_typeinfo)) {
		void (*fini)(void *);
		size_t size;
		if (!(size = info->size)
		    || off % size
		    || len % size) {
			return MPT_ERROR(BadArgument);
		}
		if ((fini = info->fini)) {
			size_t i;
			for (i = 0; i < len; i += size) {
				fini(pos + i);
			}
		}
	}
	/* move forward data after cut */
	keep -= off;
	if (keep) {
		memmove(pos, pos + len, keep);
		off += keep;
	}
	buf->_used = off;
	return off;
}
