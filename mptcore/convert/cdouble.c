/*!
 * get double value from string.
 */

#include <ctype.h>
#include <stdlib.h>

#include "convert.h"

extern int mpt_cdouble(double *val, const char *src, const double *range)
{
	double tmp;
	char *end;
	
	if (!src) {
		return MPT_ERROR(BadArgument);
	}
	if (!*src) {
		return 0;
	}
	tmp = strtod(src, &end);
	
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
}
