/*!
 * check element name against conditions.
 */

#include <errno.h>
#include <string.h>

#include "parse.h"

extern int mpt_parse_ncheck(const char *name, size_t len, int take)
{
	size_t i;
	
	/* name required */
	if (!len) {
		if (!(take & MPT_NAMEFLAG(Empty))) {
			errno = ERANGE;
			return -2;
		}
		return 0;
	}
	if (!name) {
		errno = EFAULT;
		return -1;
	}
	for (i = 0; i < len; i++) {
		/* deny whitepace */
		if (isspace(name[i])) {
			if (!(take & MPT_NAMEFLAG(Space))) {
				errno = EINVAL;
				return -3;
			}
			continue;
		}
		/* deny numerals */
		if (isdigit(name[i])) {
			if (!(take & (i ? MPT_NAMEFLAG(NumCont) : MPT_NAMEFLAG(NumStart)))) {
				errno = EINVAL;
				return -3;
			}
			continue;
		}
		/* deny binary characters */
		if (!isprint(name[i])) {
			if (!(take & MPT_NAMEFLAG(Binary))) {
				errno = EINVAL;
				return -3;
			}
			continue;
		}
		/* deny special characters */
		if (!isalnum(name[i])) {
			if (!(take & MPT_NAMEFLAG(Special))) {
				errno = EINVAL;
				return -3;
			}
			continue;
		}
	}
	return 0;
}
