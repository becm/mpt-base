/*!
 * get float value from string.
 */

#include <ctype.h>
#include <stdlib.h>

#include "convert.h"


/*!
 * \ingroup mptConvert
 * \brief read float value
 * 
 * Convert string to single precision
 * floating point value inside
 * (optional) value range.
 * 
 * \return consumed length
 */
extern int mpt_cfloat(float *val, const char *src, const float *range)
{
#if _XOPEN_SOURCE >= 600 || _ISOC99_SOURCE || _POSIX_C_SOURCE >= 200112L
	float tmp;
	char *end;
	
	if (!src) {
		return MPT_ERROR(BadArgument);
	}
	if (!*src) {
		return 0;
	}
	tmp = strtof(src, &end);
	
	if (end == src) {
		/* accept space as empty string */
		while (*src) {
			if (!isspace(*src++)) {
				return MPT_ERROR(BadType);
			}
		}
		return 0;
	}
	if (range && (range[0] > tmp || tmp > range[1])) {
		return MPT_ERROR(BadValue);
	}
	if (val) {
		*val = tmp;
	}
	return end - src;
#else
	double tmp, rf[2] = { 0, 0 };
	int ret;
	
	if (range) {
		rf[0] = range[0];
		rf[1] = range[1];
	}
	
	if ((ret = mpt_cdouble(&tmp, src, range ? rf : 0)) > 0 && val) {
		*val = tmp;
	}
	return ret;
#endif
}
