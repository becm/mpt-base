/*!
 * get position, size and offset of data types
 */

#include <ctype.h>
#include <string.h>

#include "convert.h"

/*!
 * \ingroup mptConvert
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
	int pos;
	
	if (!val->fmt) {
		pos = strcmp(val->ptr, cmp);
	}
	else if ((pos = mpt_offset(val->fmt, -1)) <= 0) {
		return pos;
	}
	else if (!(pos = memcmp(val->ptr, cmp, pos))) {
		return pos;
	}
	return pos < 0 ? -pos : pos;
}

