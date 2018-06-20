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
	MPT_STRUCT(value_format) fmt;
	const char *pos = base;
	int curr;
	
	while ((curr = mpt_valfmt_get(&fmt, pos)) > 0) {
		int err;
		if ((err = mpt_valfmt_add(arr, fmt)) < 0) {
			return err;
		}
		pos += curr;
	}
	return curr ? curr : pos - base;
}
