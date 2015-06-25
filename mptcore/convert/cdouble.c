/*!
 * get double value from string.
 */

#include <stdlib.h>
#include <errno.h>

#include "convert.h"

extern int mpt_cdouble(double *val, const char *src, const double *range)
{
	double tmp;
	char *end;
	
	if (!src) {
		errno = EFAULT;
		return MPT_ENUM(BadArgument);
	}
	if (!*src) {
		return 0;
	}
	tmp = strtod(src, &end);
	
	if (end == src) {
		return MPT_ENUM(BadType);
	}
	if (range && (range[0] > tmp || tmp > range[1])) {
		errno = ERANGE;
		return MPT_ENUM(BadValue);
	}
	if (val) {
		*val = tmp;
	}
	return end - src;
}
