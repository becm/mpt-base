/*!
 * initialize solver from shared library.
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "client.h"

/*!
 * \ingroup mptLoader
 * \brief proxy reference types
 * 
 * Create metatype proxy to library instance.
 * 
 * \param descr type description for type
 * \param end   extended description for type
 * 
 * \return type id for description
 */
extern int mpt_proxy_typeid(const char *desc, const char **end)
{
	const char *sep;
	int type = -1;
	
	if (!desc) {
		return MPT_ERROR(BadArgument);
	}
	/* no separator befor symbol end */
	if ((sep = strchr(desc, ':'))) {
		const char *to = sep++;
		if (!(type = to - desc)) {
			return MPT_ERROR(BadValue);
		}
		while (isspace(*--to)) {
			if (!(--type)) {
				return MPT_ERROR(BadValue);
			}
		}
	}
	if ((type = mpt_valtype_id(desc, type)) < 0) {
		return MPT_ERROR(BadValue);;
	}
	if (!end) {
		return type;
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
	return type;
}
