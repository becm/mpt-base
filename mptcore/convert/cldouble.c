
#include <errno.h>
#include <stdlib.h>

#include "convert.h"

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
		errno = EFAULT;
		return MPT_ENUM(BadArgument);
	}
	if (!*src) {
		return 0;
	}
	tmp = strtold(src, &end);
	
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
