
#include <ctype.h>
#include <stdlib.h>

#include "convert.h"

#ifdef _MPT_FLOAT_EXTENDED_H
/*!
 * \ingroup mptConvert
 * \brief get keyword from string
 * 
 * get keyword start and consumed length
 * until keyword end
 * 
 * \return consumed length
 */
extern int mpt_cldouble(long double *val, const char *src, const long double range[2])
{
	long double tmp;
	char *end;
	
	if (!src) {
		return MPT_ERROR(BadArgument);
	}
	if (!*src) {
		return 0;
	}
	tmp = strtold(src, &end);
	
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
#endif
