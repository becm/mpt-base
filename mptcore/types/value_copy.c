/*!
 * get position, size and offset of data types
 */

#include <ctype.h>
#include <string.h>

#include "types.h"

/*!
 * \ingroup mptTypes
 * \brief copy basic value data
 * 
 * Copy value data to target.
 * 
 * \param src   source value data
 * \param dest  target data address
 * \param max   maximum allowd data size
 * 
 * \return number of copied bytes
 */
extern ssize_t mpt_value_copy(const MPT_STRUCT(value) *src, void *dest, size_t max)
{
	const MPT_STRUCT(type_traits) *traits;
	
	/* only allow types with known traits */
	if (!(traits = mpt_type_traits(src->_type))) {
		return MPT_ERROR(BadArgument);
	}
	/* copy only valid for non-complex types */
	if (traits->init || traits->fini) {
		return MPT_ERROR(BadType);
	}
	if (traits->size > max) {
		return MPT_ERROR(MissingBuffer);
	}
	if (dest) {
		if (src->_addr) {
			memcpy(dest, src->_addr, traits->size);
		}
		else {
			memset(dest, 0, traits->size);
		}
	}
	return traits->size;
}
