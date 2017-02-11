/*!
 * encode value output format
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "convert.h"

/*!
 * \ingroup mptConvert
 * \brief parse value format
 * 
 * Set value format from description string.
 * 
 * \param arr  value format element
 * \param src  format descriptions
 * 
 * \return consumed length
 */
extern int mpt_valfmt_get(MPT_STRUCT(valfmt) *ptr, const char *src)
{
	MPT_STRUCT(valfmt) fmt = MPT_VALFMT_INIT;
	const char *pos;
	char *next;
	long val;
	
	if (!(pos = src)) {
		*ptr = fmt;
		return 0;
	}
	while (isspace(*pos)) {
		pos++;
	}
	/* select format */
	if (!*pos) {
		*ptr = fmt;
		return pos - src;
	}
	if (*pos == '+') {
		fmt.fmt |= MPT_VALFMT(Sign);
		++pos;
	}
	
	switch (tolower(*pos)) {
	  case 'f': fmt.fmt |= 6;
	  case 'g': ++pos; break;
	  case 'a': fmt.fmt |= MPT_VALFMT(Scientific);
	  case 'x': fmt.fmt |= MPT_VALFMT(NumberHex); ++pos; break;
	  case 'o': fmt.fmt |= MPT_VALFMT(IntOctal);
	  case 'e': fmt.fmt |= MPT_VALFMT(Scientific); ++pos; break;
	  default:
		if (!isdigit(*pos)) {
			return MPT_ERROR(BadArgument);
		}
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
	fmt.wdt = val;
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
	if (val < 0 || val > MPT_VALFMT_DECMAX) {
		return MPT_ERROR(BadValue);
	}
	fmt.fmt = (fmt.fmt & 0xff00) + val;
	
	*ptr = fmt;
	
	return pos - src;
}
