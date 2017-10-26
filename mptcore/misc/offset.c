/*!
 * get position, size and offset of data types
 */

#include <ctype.h>

#include "convert.h"

/*!
 * \ingroup mptConvert
 * \brief data offset
 * 
 * Get offset for data position
 * 
 * \param fmt data format description
 * \param pos position of element
 * 
 * \return data offset for element position
 */
extern int mpt_offset(const char *fmt, int pos)
{
	size_t off = 0;
	
	if (pos < 0) return -2;
	
	while (pos--) {
		int	curr;
		
		if ((curr = mpt_valsize(*fmt)) > 0) off += curr;
		else if (!curr) off += sizeof(void *);
		else if (!isspace(*fmt)) return -1;
		++fmt;
	}
	return off;
}

