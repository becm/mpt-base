/*!
 * get element in array
 */

#include "array.h"

/*!
 * \ingroup mptArray
 * \brief get array element
 * 
 * Get address of valid array element
 * assuming elements are of specified size.
 * 
 * \param arr	data array
 * \param pos	element in array
 * \param len	size of single element
 * 
 * \return start address of array element
 */
extern void *mpt_array_data(const MPT_STRUCT(array) *arr, size_t pos, size_t len)
{
	MPT_STRUCT(buffer) *b;
	
	if (!len) return 0;
	
	if (!(b = arr->_buf) || pos >= b->used/len) {
		return 0;
	}
	
	return ((uint8_t *) (b+1)) + pos * len;
}
