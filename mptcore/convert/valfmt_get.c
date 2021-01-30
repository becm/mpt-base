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
extern int mpt_valfmt_get(MPT_STRUCT(value_format) *ptr, const char *src)
{
	MPT_STRUCT(value_format) fmt = MPT_VALFMT_INIT;
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
		fmt.flags |= MPT_VALFMT(Sign);
		++pos;
	}
	
	switch (tolower(*pos)) {
	  case 'f': fmt.dec = 6; /* fall through */
	  case 'g': ++pos; break;
	  case 'a': fmt.flags |= MPT_VALFMT(Scientific); /* fall through */
	  case 'x': fmt.flags |= MPT_VALFMT(NumberHex); ++pos; break;
	  case 'o': fmt.flags |= MPT_VALFMT(IntOctal); /* fall through */
	  case 'e': fmt.flags |= MPT_VALFMT(Scientific); ++pos; break;
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
	if (val < 0 || val >= INT8_MAX) {
		return MPT_ERROR(BadValue);
	}
	fmt.dec = val;
	
	*ptr = fmt;
	
	return pos - src;
}
