/*!
 * get key value (= non space text segment) from string.
 */

#include <ctype.h>
#include <string.h>
#include <errno.h>

#include "convert.h"

/*!
 * \ingroup mptConvert
 * \brief get keyword from string
 * 
 * Get keyword start and consumed length
 * until keyword end
 * 
 * \param      src  current string position
 * \param      sep  separation characters
 * \param[out] klen pointer to length of keyword
 * 
 * \return start of keyword
 */
extern const char *mpt_convert_key(const char **src, const char *sep, size_t *klen)
{
	const char *end, *key;
	size_t len = 0;
	char curr;
	
	if (!(end = *src)) {
		return 0;
	}
	while ((curr = *end) && isspace(curr)) ++end;
	
	key = end;
	
	if (sep) {
		const char *s = sep;
		int match = 0;
		
		while (*s) { if (isspace(*s++)) { match = 1; break; } }
		
		while ((curr = *end)) {
			/* break on ANY space character */
			if (isspace(curr)) {
				if (match) {
					break;
				}
				++end;
				continue;
			}
			/* character is separator */
			else if (strchr(sep, curr)) {
				++end;
				break;
			}
			len = ++end - key;
		}
	}
	else while ((curr = *end)) {
		if (isspace(curr)) break;
		++end;
		++len;
	}
	
	if (end == key) {
		return 0;
	}
	if (klen) *klen = len;
	*src = end;
	
	return key;
}

