
#include <string.h>
#include <strings.h>

#include "object.h"

/*!
 * \ingroup mptCore
 * \brief get index of property
 * 
 * Find index of property with passed name
 * 
 * \param match property of current type
 * \param sub   properties to search
 * \param len   number of properties
 * 
 * \return index of requested property
 */
extern int mpt_property_match(const char *match, int mlen, const char * const *sub, size_t len)
{
	size_t pos = 0;
	
	if (!match) {
		return MPT_ERROR(BadArgument);
	}
	/* find property name */
	pos = 0;
	while (pos < len) {
		const char *curr = *sub;
		/* full match */
		if (mlen < 0) {
			if (!strcasecmp(match, curr)) {
				return pos;
			}
		}
		/* partial match */
		else if (!strncasecmp(match, curr, mlen)) {
			/* check for non-unique match */
			if (mlen <= (int) strlen(curr)) {
				size_t tmp = pos;
				while (++tmp < len) {
					const char *cmp = *(++sub);
					if (!strncasecmp(match, cmp, mlen)) {
						return MPT_ERROR(BadType);
					}
				}
			}
			return pos;
		}
		++sub;
		++pos;
	}
	/* name not in properties */
	return MPT_ERROR(BadValue);
}
