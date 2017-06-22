/*!
 * read until 'visible' character is found.
 */

#include <string.h>

#include "convert.h"

#include "meta.h"

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
extern int mpt_value_read(MPT_STRUCT(value) *val, const char *fmt, void *dest)
{
	const char *src, *desc;
	int len;
	
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
		int curr;
		while (*fmt && (curr = mpt_convert_string(src, *fmt, dest)) > 0) {
			++fmt;
			++len;
			src += curr;
		}
		val->ptr = src;
		return len;
	}
	while (desc[len] == fmt[len] && fmt[len]) {
		++len;
	}
	if (len) {
		int advance = mpt_offset(desc, len);
		if (advance < 0) {
			return MPT_ERROR(BadValue);
		}
		if (advance && dest) {
			memcpy(dest, src, advance);
			dest = ((uint8_t *) dest) + advance;
		}
		val->fmt += len;
		val->ptr = src + advance;
	}
	return len;
}
