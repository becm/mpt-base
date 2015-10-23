/*!
 * parse value output format
 */

#include "array.h"

#include "convert.h"

/*!
 * \ingroup mptConvert
 * \brief parse value format elements
 * 
 * Append value formats in string.
 * 
 * \param arr   value format target array
 * \param base  format descriptions
 * 
 * \return consumed length
 */
extern int mpt_valfmt_parse(MPT_STRUCT(array) *arr, const char *base)
{
	MPT_STRUCT(valfmt) fmt;
	const char *pos = base;
	int curr;
	
	while ((curr = mpt_valfmt_get(&fmt, pos)) > 0) {
		if (!mpt_array_append(arr, sizeof(fmt), &fmt)) {
			break;
		}
		pos += curr;
	}
	return pos - base;
}
