/*!
 * select data type from description
 */

#include <string.h>
#include <strings.h>
#include <ctype.h>

#include <errno.h>

#include "values.h"

/*!
 * \ingroup mptValues
 * \brief profile type
 * 
 * Get type of profile to generate.
 * 
 * \param[in]  start  profile description
 * \param[out] end    end of consumed string
 * 
 * \return type of profile
 */
extern int mpt_valtype_select(const char **end)
{
	const char *start;
	int type, len;
	
	if (!end || !(start = *end) || *start == ':') {
		return MPT_ENUM(ValueFormatLinear);
	}
	
	while (*start && isspace(*start)) {
		start++;
	}
	if (!strncasecmp(start, "lin", len = 3)) {
		type = MPT_ENUM(ValueFormatLinear);
	} else if (!strncasecmp(start, "bound", len = 5)) {
		type = MPT_ENUM(ValueFormatBoundaries);
	} else if (!strncasecmp(start, "poly", len = 4)) {
		type = MPT_ENUM(ValueFormatPolynom);
	} else if (!strncasecmp(start, "val", len = 3)) {
		type = MPT_ENUM(ValueFormatText);
	} else if (!strncasecmp(start, "file", len = 4)) {
		type = MPT_ENUM(ValueFormatFile);
	} else {
		return MPT_ERROR(BadArgument);
	}
	if (!start[len]) {
		*end = start + len;
	}
	else if (start[len] == ':' || isspace(start[len])) {
		*end = start + len + 1;
	}
	else {
		return MPT_ERROR(BadValue);
	}
	return type;
}

