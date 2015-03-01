/*!
 * get position, size and offset of data types
 */

#include <string.h>

#include "convert.h"

/*!
 * \ingroup mptConvert
 * \brief type position
 * 
 * Get type position in data description
 * 
 * \param fmt  data format description
 * \param type type of element
 * 
 * \return position of element
 */
extern int mpt_position(const char *fmt, int type)
{
	int pos = 0;
	
	if (type & ~0xff) return -2;
	
	while (fmt[pos]) {
		if (fmt[pos] == type) return pos;
		switch (fmt[pos]) {
		  case 'b': case 'c':		if (strchr("cb",  type)) return pos; break;
		  case 'B': case 'C':		if (strchr("CB",  type)) return pos; break;
		  case 'o': case 'x': case 'I':	if (strchr("oxI", type)) return pos; break;
		  case 'O': case 'X': case 'L':	if (strchr("OXL", type)) return pos; break;
		  case 'f': case 'g': 		if (strchr("fg",  type)) return pos; break;
		  case 'd': case 'F': case 'G': if (strchr("dFG", type)) return pos; break;
		  default:;
		}
		++pos;
	}
	return -1;
}
