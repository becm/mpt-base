/*!
 * remove data from array
 */

#include <string.h>

#include "array.h"

/*!
 * \ingroup mptArray
 * \brief remove array data
 * 
 * Cut data from array starting from offset.
 * 
 * \param arr	data array
 * \param off	start position to remove
 * \param len	length of element to remove
 * 
 * \return size of moved data
 */
ssize_t mpt_array_cut(MPT_STRUCT(array) *arr, size_t off, size_t len)
{
	MPT_STRUCT(buffer) *buf;
	size_t	move;
	
	if (!(buf = arr->_buf)) return -1;
	
	if (len > buf->used) return -3;
	
	if ((move = buf->used - len) < off) return -2;
	
	buf->used = move;
	if ((move -= off)) {
		uint8_t *pos = ((uint8_t *) (buf+1)) + off;
		memmove(pos, pos + len, move);
	}
	return move;
}
