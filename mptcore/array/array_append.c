/*!
 * array extension
 */

#include <errno.h>
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
	size_t used;
	void *dest;
	
	if (!len) return 0;
	
	if (!(b = arr->_buf)) {
		if (!(b = _mpt_buffer_alloc(len))) {
			return 0;
		}
		used = 0;
		arr->_buf = b;
	}
	/* require raw buffer to append data */
	else if (b->_vptr->content(b)) {
		errno = EINVAL;
		return 0;
	}
	else if (len > (b->_size - (used = b->_used))) {
		if (len > (SIZE_MAX - used)) {
			errno = EINVAL;
			return 0;
		}
		if (!(b = b->_vptr->detach(b, used + len))) {
			return 0;
		}
		arr->_buf = b;
	}
	dest = ((uint8_t *)(b + 1)) + used;
	b->_used = used + len;
	
	if (base) {
		memcpy(dest, base, len);
	} else {
		memset(dest, 0, len);
	}
	return dest;
}
