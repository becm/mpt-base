/*!
 * get position, size and offset of data types
 */

#include <ctype.h>

#include "convert.h"

/*!
 * \ingroup mptConvert
 * \brief data offset
 * 
 * Get offset for data position.
 * Use  `match = \<0` to get total size and
 * `match = 0` to get invalid data offset.
 * 
 * \param fmt data format description
 * \param pos position of element
 * 
 * \return data offset for element position
 */
extern int mpt_offset(const uint8_t *fmt, int pos)
{
	size_t off = 0;
	int curr;
	
	while ((curr = *fmt++)) {
		int len;
		
		if (isspace(curr)) {
			continue;
		}
		if ((len = mpt_valsize(curr)) < 0) {
			if (!pos) {
				return off;
			}
			return curr;
		}
		if (!len) {
			off += sizeof(void *);
		} else {
			off += len;
		}
		if (pos > 0 && !--pos) {
			return off;
		}
	}
	if (pos < 0) {
		return off;
	}
	return MPT_ERROR(BadValue);
}

