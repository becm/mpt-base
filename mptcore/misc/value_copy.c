/*!
 * get position, size and offset of data types
 */

#include <ctype.h>
#include <string.h>

#include "types.h"

/*!
 * \ingroup mptTypes
 * \brief copy value data
 * 
 * Copy settings to target value.
 * 
 * \param dest  target value address
 * \param src   source value data
 * 
 * \return number of copied local bytes
 */
extern int mpt_value_copy(MPT_STRUCT(value) *dest, const MPT_STRUCT(value) *src)
{
	int ret = 0;
	if (src->ptr == src->_buf) {
		ret = dest->_bufsize;
		if (src->_bufsize > ret) {
			return MPT_ERROR(MissingBuffer);
		}
		if (src->_bufsize < ret) {
			ret = src->_bufsize;
		}
		dest->ptr = memcpy(dest->_buf, src->_buf, ret);
	}
	else {
		dest->ptr = src->ptr;
	}
	dest->domain = src->domain;
	dest->type = src->type;
	
	return ret;
}
