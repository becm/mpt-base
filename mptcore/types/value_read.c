/*!
 * read until 'visible' character is found.
 */

#include <string.h>

#include "types.h"

#include "convert.h"

/*!
 * \ingroup mptTypes
 * \brief next nonspace character
 * 
 * Get next visible character and advance position.
 * 
 * \param ptr  string pointer reference
 * 
 * \return found visible value
 */
extern int mpt_value_read(MPT_STRUCT(value) *val, const char *fmt, void *dest)
{
	const char *src;
	const uint8_t *desc;
	size_t take;
	int curr, len;
	
	if (!fmt) {
		return MPT_ERROR(BadArgument);
	}
	if (!*fmt) {
		return 0;
	}
	if (!(src = val->ptr)) {
		return MPT_ERROR(MissingData);
	}
	len = 0;
	if (!(desc = val->fmt)) {
		while (*fmt && (curr = mpt_convert_string(src, *fmt, dest)) > 0) {
			++fmt;
			++len;
			src += curr;
		}
		val->ptr = src;
		return len;
	}
	take = 0;
	while ((curr = fmt[len]) && curr == desc[len]) {
		const MPT_STRUCT(type_traits) *traits = mpt_type_traits(curr);
		if (!traits) {
			break;
		}
		++len;
		take += traits->size;
	}
	if (take && dest) {
		val->fmt += len;
		val->ptr = src + take;
	}
	return len;
}
