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
	
	if (match & ~0xff) {
		return MPT_ERROR(BadValue);
	}
	while ((curr = fmt[pos])) {
		char type = match;
		
		if (curr == type) {
			return pos;
		}
		/* current is vector entry */
		if (MPT_value_isVector(curr)) {
			curr = MPT_value_fromVector(curr);
			if (curr == 'c' && type == 's') {
				return pos;
			}
			/* vector types only */
			if ((type = MPT_value_isVector(type))
			    && curr == (int) type) {
				return pos;
			}
			++pos;
			continue;
		}
		/* current is array */
		if (curr == MPT_ENUM(TypeArray)) {
			/* wide match from array to vector,
			 * need deep compare for actual datatype */
			if ((curr = MPT_value_fromVector(match)) >= 0) {
				return pos;
			}
			++pos;
			continue;
		}
		switch (curr) {
		  /* accept more specific type */
		  case 'y': if (type == 'y' || type == 'u' || type == 't') return pos; break;
		  case 'b': if (type == 'b' || type == 'i' || type == 'x') return pos; break;
		  case 'u': if (type == 'u' || type == 't') return pos; break;
		  case 'i': if (type == 'i' || type == 'x') return pos; break;
		  case 'o': if (type == 'o' || type == 's') return pos; break;
		  case 'k': if (type == 'k' || type == 's') return pos; break;
		  /* exact match */
		  case 'c': if (type == 'c') return pos; break;
		  case 't': if (type == 't') return pos; break;
		  case 'x': if (type == 'x') return pos; break;
		  case 's': if (type == 's') return pos; break;
		  /* float types (accept better precision) */
		  case 'f': if (type == 'f') return pos;
		  case 'd': if (type == 'd' || type == 'f') return pos;
		  case 'e': if (type == 'e' || type == 'd' || type == 'f') return pos;
		  default:;
		}
		++pos;
	}
	return MPT_ERROR(BadType);
}
