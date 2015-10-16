/*!
 * get position, size and offset of data types
 */

#include <ctype.h>

#include "convert.h"

/*!
 * \ingroup mptConvert
 * \brief type position
 * 
 * Get type position in data description
 * 
 * \param fmt   data format description
 * \param match type of element
 * 
 * \return position of element
 */
extern int mpt_position(const char *fmt, int match)
{
	char curr;
	int pos = 0;
	
	if (match & ~0xff) return -2;
	
	while ((curr = fmt[pos])) {
		char type = match;
		
		if (curr == type) {
			return pos;
		}
		/* current is vector entry */
		if (curr & MPT_ENUM(TypeVector)) {
			curr &= 0x7f;
			if (curr == 'c' && type == 's') {
				return pos;
			}
			/* vector types only */
			if (!(type & MPT_ENUM(TypeVector))) {
				++pos;
				continue;
			}
			type &= 0x7f;
		}
		/* current is array */
		else if (isupper(curr)) {
			/* accept generic array */
			if (type == MPT_ENUM(TypeArray)) {
				return pos;
			}
			/* compatible vector type */
			if (type & MPT_ENUM(TypeVector)) {
				type &= 0x7f;
			}
			/* compatible array type */
			else if (isupper(type)) {
				type = tolower(type);
			}
			/* incompatible types */
			else {
				++pos;
				continue;
			}
			/* accept array with more generic entities */
			curr = tolower(curr);
		}
		switch (curr) {
		  /* accept more specific type */
		  case 'y': if (type == 'y' || type == 'c') return pos; break;
		  case 'b': if (type == 'b' || type == 'c') return pos; break;
		  case 'o': if (type == 'o' || type == 's') return pos; break;
		  case 'k': if (type == 'k' || type == 's') return pos; break;
		  /* exact match */
		  case 'c': if (type == 'c') return pos; break;
		  case 'x': if (type == 'x') return pos; break;
		  case 't': if (type == 't') return pos; break;
		  case 's': if (type == 's') return pos; break;
		  /* float types (accept better precision) */
		  case 'f': if (type == 'f') return pos;
		  case 'd': if (type == 'd') return pos;
		  case 'e': if (type == 'e') return pos;
		  default:;
		}
		++pos;
	}
	return -1;
}
