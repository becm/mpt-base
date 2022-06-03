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
 * \param arr     array to be updated
 * \param len     number of bytes to reserve
 * \param traits  type traits for elements
 * 
 * \return buffer satisfying requested type and space
 */
extern MPT_STRUCT(buffer) *mpt_array_reserve(MPT_STRUCT(array) *arr, size_t len, const MPT_STRUCT(type_traits) *traits)
{
	MPT_STRUCT(buffer) *buf;
	
	/* check arguments */
	if (traits) {
		size_t align;
		if (!traits->size) {
			errno = EINVAL;
			return 0;
		}
		/* total data must align with element size */
		if ((align = len % traits->size)) {
			len += traits->size - align;
		}
	}
	/* existing buffer can be processed normally */
	if ((buf = arr->_buf)) {
		const MPT_STRUCT(type_traits) *old = buf->_content_traits;
		if (traits != old) {
			void (*fini)(void *) = old ? old->fini : 0;
			size_t used;
			if (fini && (used = buf->_used)) {
				size_t pos, elem_size = old->size;
				uint8_t *ptr = (void *) (buf + 1);
				used -= used % elem_size;
				for (pos = 0; pos < used; pos += elem_size) {
					fini(ptr + pos);
				}
			}
			buf->_used = 0;
			buf->_content_traits = traits;
		}
		return mpt_array_slice(arr, 0, len) ? arr->_buf : 0;
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
	return buf;
}
