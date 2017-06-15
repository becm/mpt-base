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
 * \param arr  array descriptor
 * 
 * \return rew allocated size
 */
size_t mpt_array_reduce(MPT_STRUCT(array) *arr)
{
	MPT_STRUCT(buffer) *buf;
	size_t size;
	
	if (!(buf = arr->_buf)) {
		return 0;
	}
	size = buf->_size;
	if (buf->_ref._val > 1) {
		return size;
	}
	if (buf->_vptr->content(buf)) {
		buf = buf->_vptr->detach(buf, -1);
	} else {
		buf = buf->_vptr->detach(buf, buf->_used);
	}
	if (!buf) {
		return size;
	}
	arr->_buf = buf;
	return buf->_size;
}

/*!
 * \ingroup mptArray
 * \brief set buffer reference
 * 
 * Copy buffer reference with source array.
 * Pass source zero pointer to clear target buffer reference.
 * 
 * \param arr  target array pointer
 * \param from source array pointer
 */
void mpt_array_clone(MPT_STRUCT(array) *arr, const MPT_STRUCT(array) *from)
{
	MPT_STRUCT(buffer) *buf;
	
	if ((buf = arr->_buf)) {
		if (from && from->_buf == buf) {
			return;
		}
		buf->_vptr->ref.unref((void *) buf);
		arr->_buf = 0;
	}
	if (from
	    && (buf = from->_buf)
	    && mpt_reference_raise(&buf->_ref)) {
		arr->_buf = buf;
	}
}
