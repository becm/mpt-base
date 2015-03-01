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
		if (!(take & MPT_ENUM(NameEmpty))) {
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
			if (!(take & MPT_ENUM(NameSpace))) {
				errno = EINVAL;
				return -3;
			}
			continue;
		}
		/* deny numerals */
		if (isdigit(name[i])) {
			if (!(take & (i ? MPT_ENUM(NameNumCont) : MPT_ENUM(NameNumStart)))) {
				errno = EINVAL;
				return -3;
			}
			continue;
		}
		/* deny binary characters */
		if (!isprint(name[i])) {
			if (!(take & MPT_ENUM(NameBinary))) {
				errno = EINVAL;
				return -3;
			}
			continue;
		}
		/* deny special characters */
		if (!isalnum(name[i])) {
			if (!(take & MPT_ENUM(NameSpecial))) {
				errno = EINVAL;
				return -3;
			}
			continue;
		}
	}
	return 0;
}
