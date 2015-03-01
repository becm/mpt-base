/*!
 * array maintenance
 */

#include "array.h"

/*!
 * \ingroup mptArray
 * \brief shrink buffer data
 * 
 * Free unused data for non-shared buffer
 * 
 * \param arr	array descriptor
 * 
 * \return rew allocated size
 */
size_t mpt_array_reduce(MPT_STRUCT(array) *arr)
{
	MPT_STRUCT(buffer) *buf;
	size_t	used, size;
	
	if (!(buf = arr->_buf)) return 0;
	
	if (buf->shared || !buf->resize) return buf->size;
	
	if (!(used = buf->used)) used = 1;
	
	if (MPT_align(used) < (size = buf->size)) {
		if (!(buf = (buf->resize(buf, used))))
			return size;
		arr->_buf = buf;
		size = buf->size;
	}
	return size;
}

/*!
 * \ingroup mptArray
 * \brief set buffer reference
 * 
 * Copy buffer reference with source array.
 * Pass source zero pointer to clear target buffer reference.
 * 
 * \param arr	target array pointer
 * \param from	source array pointer
 */
void mpt_array_clone(MPT_STRUCT(array) *arr, const MPT_STRUCT(array) *from)
{
	MPT_STRUCT(buffer) *buf;
	
	if ((buf = arr->_buf)) {
		if (!buf->resize) {
			buf->shared--;
		} else {
			buf->resize(buf, 0);
		}
	}
	arr->_buf = buf = from ? from->_buf : 0;
	
	if (buf) ++buf->shared;
}
