/*!
 * encode value output format
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "output.h"

extern int mpt_outfmt_get(MPT_STRUCT(valfmt) *ptr, const char *src)
{
	static const MPT_STRUCT(valfmt) deffmt = MPT_VALFMT_INIT;
	MPT_STRUCT(valfmt) fmt;
	const char *pos;
	char *next;
	long val;
	
	if (!(pos = src)) {
		*ptr = deffmt;
		return 0;
	}
	while (isspace(*pos)) {
		pos++;
	}
	fmt = deffmt;
	
	/* select format */
	if (!*pos) {
		*ptr = fmt;
		return pos - src;
	}
	if (*pos == '+') {
		fmt.flt = -fmt.flt;
		++pos;
	}
	if (strchr("fgea", tolower(*pos))) {
		char curr = *pos++;
		fmt.flt = (fmt.flt < 0) ? -curr : curr;
	}
	else if (*pos == 'n') {
		++pos;
	}
	else if (*pos == 'N') {
		fmt.flt = toupper(fmt.flt);
		++pos;
	}
	else if (!isdigit(*pos)) {
		errno = EINVAL;
		return MPT_ERROR(BadArgument);
	}
	/* get field with */
	val = strtol(pos, &next, 0);
	
	if (next == pos) {
		return MPT_ERROR(BadArgument);
	}
	pos = next;
	if (val < 0 || val > UINT8_MAX) {
		return MPT_ERROR(BadValue);
	}
	fmt.width = val;
	if (!*pos || isspace(*pos)) {
		*ptr = fmt;
		return pos - src;
	}
	if (*(pos++) != '.') {
		errno = EINVAL;
		return -3;
	}
	/* get digits after decimal point */
	val = strtol(pos, &next, 0);
	
	if (next == pos) {
		return MPT_ERROR(BadArgument);
	}
	pos = next;
	if (val < 0 || val > INT8_MAX) {
		return MPT_ERROR(BadValue);
	}
	fmt.dec = val;
	
	*ptr = fmt;
	
	return pos - src;
}
