/*!
 * insert data in array
 */

#include <string.h>

#include "array.h"

/*!
 * \ingroup mptArray
 * \brief insert array data
 * 
 * Insert data in array on position.
 * Extend array if needed and
 * zero invalid data preceding position.
 * 
 * \param arr	array descriptor
 * \param pos	where to insert
 * \param len	inserted data length
 * 
 * \return start address added data
 */
extern void *mpt_array_insert(MPT_STRUCT(array) *arr, size_t pos, size_t len)
{
	MPT_STRUCT(buffer) *b, *(*resize)(MPT_STRUCT(buffer) *, size_t );
	uint8_t	*base;
	size_t	used, size, total;
	
	if ((b = arr->_buf)) {
		used = b->used;
		size = b->size;
		resize = b->resize;
		base = (uint8_t *) (b+1);
	} else {
		used = size = 0;
		resize = _mpt_buffer_realloc;
		base = 0;
	}
	
	if (!len) return base;
	
	total = len + pos;
	
	/* set after used space */
	if (pos >= used) {
		if (total > size) {
			if (!(b = resize ? resize(b, total) : 0))
				return 0;
			arr->_buf = b;
			base = (uint8_t *) (b+1);
		}
		b->used = total;
		if (pos > used) memset(base+used, 0, pos-used);
		return base + pos;
	}
	total = used + len;
	
	/* increase allocation size */
	if (used >= size && !(b = resize ? resize(b, total) : 0))
		return 0;
	
	/* increase used size */
	size = used - pos;
	b->used = total;
	
	/* move post data */
	base = ((uint8_t *) (b+1)) + pos;
	memmove(base + len, base, size);
	
	return base;
}
