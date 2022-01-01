/*!
 * MPT core library
 *   assign array buffer reference
 */

#include "array.h"

/*!
 * \ingroup mptArray
 * \brief free encode array resources
 * 
 * Clear encoding context and buffer reference.
 * 
 * \param arr  encoding array pointer
 */
void mpt_encode_array_fini(MPT_STRUCT(encode_array) *arr)
{
	MPT_STRUCT(buffer) *buf;
	
	if (arr->_enc) {
		arr->_enc(&arr->_state, 0, 0);
	}
	
	if ((buf = arr->_d._buf)) {
		buf->_vptr->unref(buf);
	}
}
