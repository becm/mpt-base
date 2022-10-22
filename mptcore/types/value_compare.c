/*!
 * get position, size and offset of data types
 */

#include <ctype.h>
#include <string.h>

#include "types.h"

/*!
 * \ingroup mptTypes
 * \brief data offset
 * 
 * Get offset for data position
 * 
 * \param val typed value reference
 * \param cmp data of same type
 * 
 * \return data offset for element position
 */
extern int mpt_value_compare(const MPT_STRUCT(value) *val, const void *cmp)
{
	const MPT_STRUCT(type_traits) *traits;
	int pos;
	
	if (!MPT_value_isBaseType(val) || !(traits = mpt_type_traits(val->_type))) {
		return MPT_ERROR(BadType);
	}
	if (!(pos = traits->size)) {
		return 0;
	}
	pos = memcmp(val->_addr, cmp, pos);
	
	return pos < 0 ? -pos : pos;
}

