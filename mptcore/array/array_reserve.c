/*!
 * MPT core library
 *   reserve typed data on array
 */

#include <errno.h>

#include "types.h"

#include "array.h"

/*!
 * \ingroup mptCore
 * \brief reserve data on array
 * 
 * Prepare elements on array.
 * 
 * \param arr     array to append data
 * \param len     number of elements
 * \param traits  type traits for elements
 * 
 * \return start address of reserved data
 */
extern void *mpt_array_reserve(MPT_STRUCT(array) *arr, long len, const MPT_STRUCT(type_traits) *traits)
{
	MPT_STRUCT(buffer) *buf;
	
	/* check arguments */
	if (!traits || !traits->size || len < 0 || (SIZE_MAX / traits->size) < (size_t) len) {
		errno = EINVAL;
		return 0;
	}
	/* length is total elements times element size */
	len *= traits->size;
	
	/* existing buffer can be processed normally */
	if ((buf = arr->_buf)) {
		return mpt_array_slice(arr, 0, len);
	}
	/* create new buffer */
	if (!(buf = _mpt_buffer_alloc(len, 0))) {
		return 0;
	}
	buf->_content_traits = traits;
	/* initialize new elements */
	if (mpt_buffer_set(buf, traits, len, 0, 0) < 0) {
		buf->_vptr->unref(buf);
		return 0;
	}
	arr->_buf = buf;
	return buf + 1;
}
