/*!
 * get float value from string.
 */

#include <stdlib.h>
#include <errno.h>

#include "convert.h"

#if _XOPEN_SOURCE >= 600 || _ISOC99_SOURCE || _POSIX_C_SOURCE >= 200112L

extern int mpt_cfloat(float *val, const char *src, const float *range)
{
	float tmp;
	char *end;
	
	if (!src) {
		errno = EFAULT;
		return MPT_ERROR(BadArgument);
	}
	if (!*src) {
		return 0;
	}
	tmp = strtof(src, &end);
	
	if (end == src) {
		return MPT_ERROR(BadType);
	}
	if (range && (range[0] > tmp || tmp > range[1])) {
		errno = ERANGE;
		return MPT_ERROR(BadValue);
	}
	if (val) {
		*val = tmp;
	}
	return end - src;
}
#else /* has(strtof) */

#include <math.h>

extern int mpt_cfloat(float *val, const char *src, const float *range)
{
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
}
#endif /* has(strtof) */
