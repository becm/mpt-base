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
