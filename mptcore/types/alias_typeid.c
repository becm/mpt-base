/*!
 * initialize solver from shared library.
 */

#include <ctype.h>
#include <string.h>

#include "types.h"

/*!
 * \ingroup mptTypes
 * \brief proxy reference types
 * 
 * Create metatype proxy to library instance.
 * 
 * \param desc  description for symbol alias
 * \param end   extended description for type
 * 
 * \return type id for description
 */
extern int mpt_alias_typeid(const char *desc, const char **end)
{
	const MPT_STRUCT(named_traits) *traits;
	const char *sep;
	int len = -1;
	
	if (!desc) {
		return MPT_ERROR(BadArgument);
	}
	/* find separator before symbol start */
	if ((sep = strchr(desc, ':'))) {
		const char *to = sep++;
		if (!(len = to - desc)) {
			return MPT_ERROR(BadValue);
		}
		while (isspace(*--to)) {
			if (!(--len)) {
				return MPT_ERROR(BadValue);
			}
		}
	}
	if (!(traits = mpt_named_traits(desc, len))) {
		return MPT_ERROR(BadValue);
	}
	if (!end) {
		return traits->type;
	}
	if (sep) {
		char curr;
		while ((curr = *sep) && isspace(curr)) {
			++sep;
		}
		*end = sep;
	} else {
		*end = desc + strlen(desc);
	}
	return traits->type;
}
