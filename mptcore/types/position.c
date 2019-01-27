/*!
 * get position, size and offset of data types
 */

#include <ctype.h>

#include "types.h"

/*!
 * \ingroup mptTypes
 * \brief match position
 * 
 * Get type position in data description.
 * Use  `match = \<0` to get length and
 * `match = 0` to get invalid format position.
 * 
 * \param fmt   data format description
 * \param match type of element
 * 
 * \return position of element
 */
extern int mpt_position(const uint8_t *fmt, int match)
{
	int curr, pos = 0;
	
	if (match < 0) {
		while (*fmt++) {
			++pos;
		}
		return pos;
	}
	if (!match) {
		while ((curr = fmt[pos])) {
			if (!isspace(curr)
			    && (curr = mpt_valsize(curr)) < 0) {
				return pos;
			}
			++pos;
		}
		return MPT_ERROR(BadValue);
	}
	if (match > 0xff) {
		return MPT_ERROR(BadValue);
	}
	while ((curr = fmt[pos])) {
		int stype;
		
		if (curr == match) {
			return pos;
		}
		if (isspace(curr)) {
			++pos;
			continue;
		}
		/* current is vector entry */
		if ((stype = MPT_type_fromVector(curr)) >= 0) {
			if (stype == 'c' && match == 's') {
				return pos;
			}
			/* identical/genertic target type */
			curr = MPT_type_fromVector(match);
			if (!curr || curr == stype) {
				return pos;
			}
			++pos;
			continue;
		}
		/* current is array */
		if (curr == MPT_ENUM(TypeArray)) {
			/* wide match from array to vector,
			 * need deep compare for actual datatype */
			if ((curr = MPT_type_fromVector(match)) >= 0) {
				return pos;
			}
			++pos;
			continue;
		}
		switch (curr) {
		  /* accept more specific type */
		  case 'y': if (match == 'y' || match == 'u' || match == 't') return pos; break;
		  case 'b': if (match == 'b' || match == 'i' || match == 'x') return pos; break;
		  case 'u': if (match == 'u' || match == 't') return pos; break;
		  case 'i': if (match == 'i' || match == 'x') return pos; break;
		  case 'o': if (match == 'o' || match == 's') return pos; break;
		  case 'k': if (match == 'k' || match == 's') return pos; break;
		  /* exact match */
		  case 'c': if (match == 'c') return pos; break;
		  case 't': if (match == 't') return pos; break;
		  case 'x': if (match == 'x') return pos; break;
		  case 's': if (match == 's') return pos; break;
		  /* float types (accept better precision) */
		  case 'f': if (match == 'f') return pos;
		  case 'd': if (match == 'd' || match == 'f') return pos;
		  case 'e': if (match == 'e' || match == 'd' || match == 'f') return pos;
		  default:;
		}
		++pos;
	}
	return MPT_ERROR(BadType);
}
