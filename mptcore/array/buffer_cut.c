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
	size_t move;
	
	if (len > buf->_used) {
		return MPT_ERROR(BadArgument);
	}
	if ((move = buf->_used - len) < off) {
		return MPT_ERROR(MissingData);
	}
	if (!len) {
		buf->_used = off;
		return 0;
	}
	buf->_used = move;
	move -= off;
	if ((move -= off)) {
		uint8_t *pos = ((uint8_t *) (buf+1)) + off;
		memmove(pos, pos + len, move);
	}
	return move;
}
