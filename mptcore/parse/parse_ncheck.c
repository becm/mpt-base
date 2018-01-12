/*!
 * check element name against conditions.
 */

#include <string.h>
#include <ctype.h>

#include "parse.h"

extern int mpt_parse_ncheck(const char *name, size_t len, int take)
{
	size_t i;
	
	/* name required */
	if (!len) {
		if (!(take & MPT_NAMEFLAG(Empty))) {
			return MPT_ERROR(MissingData);
		}
		return 0;
	}
	if (!name) {
		return MPT_ERROR(BadArgument);
	}
	for (i = 0; i < len; i++) {
		/* deny whitepace */
		if (isspace(name[i])) {
			if (!(take & MPT_NAMEFLAG(Space))) {
				return MPT_ERROR(BadType);
			}
			continue;
		}
		/* deny numerals */
		if (isdigit(name[i])) {
			if (!(take & (i ? MPT_NAMEFLAG(NumCont) : MPT_NAMEFLAG(NumStart)))) {
				return MPT_ERROR(BadValue);
			}
			continue;
		}
		/* deny binary characters */
		if (!isprint(name[i])) {
			if (!(take & MPT_NAMEFLAG(Binary))) {
				return MPT_ERROR(BadValue);
			}
			continue;
		}
		/* deny special characters */
		if (!isalnum(name[i])) {
			if (!(take & MPT_NAMEFLAG(Special))) {
				return MPT_ERROR(BadValue);
			}
			continue;
		}
	}
	return 0;
}
