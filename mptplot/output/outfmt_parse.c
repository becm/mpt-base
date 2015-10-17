/*!
 * parse value output format
 */

#include "array.h"

#include "output.h"

extern int mpt_outfmt_parse(MPT_STRUCT(array) *arr, const char *base)
{
	MPT_STRUCT(valfmt) fmt;
	const char *pos = base;
	int curr;
	
	while ((curr = mpt_outfmt_get(&fmt, pos)) > 0) {
		if (!mpt_array_append(arr, sizeof(fmt), &fmt)) {
			break;
		}
		pos += curr;
	}
	return pos - base;
}
