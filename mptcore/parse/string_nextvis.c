/*!
 * read until 'visible' character is found.
 */

#include <string.h>

#include "parse.h"

/*!
 * \ingroup mptParse
 * \brief next nonspace character
 * 
 * Get next visible character and advance position.
 * 
 * \param ptr  string pointer reference
 * 
 * \return found visible value
 */
extern int mpt_string_nextvis(const char **pos)
{
	const char *str;
	int curr;
	
	if (!pos) {
		return MPT_ERROR(BadArgument);
	}
	if (!(str = *pos) || !(curr = *str)) {
		return MPT_ERROR(MissingData);
	}
	while ((isspace(curr))) {
		if (!(curr = *(++str))) {
			return MPT_ERROR(MissingData);
		}
		if (!isgraph(curr)) {
			return MPT_ERROR(BadValue);
		}
	}
	*pos = str;
	return curr;
}
