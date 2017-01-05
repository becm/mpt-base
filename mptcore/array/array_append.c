/*!
 * array extension
 */

#include <string.h>

#include "array.h"

/*!
 * \ingroup mptArray
 * \brief append data to array
 * 
 * Push data to array, resize if needed.
 * Use data null pointer to add zeroed data.
 * 
 * \param arr  array descriptor
 * \param len  length to append
 * \param base data to append
 * 
 * \return start address appended data
 */
extern void *mpt_array_append(MPT_STRUCT(array) *arr, size_t len, const void *base)
{
	MPT_STRUCT(buffer) *b;
	size_t	used;
	void	*dest;
	
	if (!len) return 0;
	
	if (!(b = arr->_buf)) {
		if (!(b = _mpt_buffer_realloc(0, sizeof(*b)+len))) {
			return 0;
		}
		b->used = len;
		used = 0;
		arr->_buf = b;
	}
	else if (len > (b->size - (used=b->used))) {
		if (!b->resize || !(b = b->resize(b, used+len))) {
			return 0;
		}
		arr->_buf = b;
	}
	dest = ((uint8_t *)(b+1)) + used;
	b->used = used + len;
	
	if (base) {
		memcpy(dest, base, len);
	} else {
		memset(dest, 0, len);
	}
	return dest;
}
