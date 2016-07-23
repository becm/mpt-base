/*!
 * get value from string
 */

#include <ctype.h>
#include <stddef.h>
#include <string.h>

#include "convert.h"

/*!
 * \ingroup mptConvert
 * \brief number from string
 * 
 * Convert text data to simple number type.
 * 
 * \param src  source text
 * \param fmt  target dype
 * \param dest target data
 * 
 * \return consumed length
 */
extern int mpt_convert_number(const char *src, int fmt, void *dest)
{
	if (!src) {
		return 0;
	}
	if (fmt == 'c') {
		const char *pos = src;
		while (isspace(*pos)) ++pos;
		if (!*pos) return 0;
		if (!isgraph(*pos)) return MPT_ERROR(BadType);
		if (dest) *((char *) dest) = *pos;
		return pos + 1 - src;
	}
	if (fmt == 'l') {
		fmt = mpt_typeidentifier_long();
	}
	switch (fmt) {
		case 'b': return mpt_cint8 (dest, src, 0, 0);
		case 'y': return mpt_cuint8(dest, src, 0, 0);
		
		case 'n': return mpt_cint16 (dest, src, 0, 0);
		case 'q': return mpt_cuint16(dest, src, 0, 0);
		
		case 'i': return mpt_cint32 (dest, src, 0, 0);
		case 'u': return mpt_cuint32(dest, src, 0, 0);
		
		case 'x': return mpt_cuint64(dest, src,  0, 0);
		case 't': return mpt_cuint64(dest, src,  0, 0);
		case 'l': return mpt_cuint64(dest, src, 16, 0);
		
		case 'f': return mpt_cfloat(dest, src, 0);
		case 'd': return mpt_cdouble(dest, src, 0);
		case 'e': return mpt_cldouble(dest, src, 0);
		
		default: return MPT_ERROR(BadType);
	}
}
