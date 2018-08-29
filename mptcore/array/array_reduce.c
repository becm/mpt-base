/*!
 * MPT core library
 *   array maintenance
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
	if (!(buf = buf->_vptr->detach(buf, buf->_used))) {
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
 * 
 * \retval mpt::BadOperation  unable to create clone
 * \retval 0  no change
 * \retval 1  new buffer assigned
 * \retval 2  buffer removed
 * \retval 3  existing buffer replaced
 */
int mpt_array_clone(MPT_STRUCT(array) *arr, const MPT_STRUCT(array) *from)
{
	MPT_STRUCT(buffer) *buf = arr->_buf, *set = 0;
	
	if (from) {
		set = from->_buf;
		if (set == buf) {
			return 0;
		}
		if (!set->_vptr->instance.addref((void *) set)) {
			return MPT_ERROR(BadOperation);
		}
	}
	arr->_buf = set;
	if (buf) {
		buf->_vptr->instance.unref((void *) buf);
		return set ? 3 : 2;
	}
	return set ? 1 : 0;
}
