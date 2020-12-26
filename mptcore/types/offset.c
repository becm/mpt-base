/*!
 * get position, size and offset of data types
 */

#include <ctype.h>

#include "types.h"

/*!
 * \ingroup mptTypes
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
	
	if (!pos) {
		return 0;
	}
	while ((curr = *fmt++)) {
		const MPT_STRUCT(type_traits) *traits;
		
		if (isspace(curr)) {
			continue;
		}
		if (!(traits = mpt_type_traits(curr))) {
			if (!pos) {
				return off;
			}
			return MPT_ERROR(BadType);
		}
		off += traits->size;
		
		if (pos > 0 && !--pos) {
			return off;
		}
	}
	if (pos < 0) {
		return off;
	}
	return MPT_ERROR(BadValue);
}

