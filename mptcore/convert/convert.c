/*!
 * get value from string
 */

#include <stddef.h>
#include <string.h>
#include <errno.h>

#include "layout.h"
#include "convert.h"

/*!
 * \ingroup mptConvert
 * \brief type from string
 * 
 * Convert text data to simple type.
 * 
 * \param src  source text
 * \param fmt  target dype
 * \param dest target data
 * 
 * \return consumed length
 */
extern int mpt_convert(const char *src, int fmt, void *dest)
{
	if (!src) {
		return 0;
	}
	switch (fmt) {
		case 'c': return mpt_cint8 (dest, src,  0, 0);
		case 'b': return mpt_cint8 (dest, src,  16, 0);
		case 'C': return mpt_cuint8(dest, src,  0, 0);
		case 'B': return mpt_cuint8(dest, src,  16, 0);
		
		case 'h': return mpt_cint16 (dest, src,  0, 0);
		case 'H': return mpt_cuint16(dest, src,  0, 0);
		
		case 'i': return mpt_cint32 (dest, src,  0, 0);
		case 'o': return mpt_cuint32(dest, src,  8, 0);
		case 'x': return mpt_cuint32(dest, src, 16, 0);
		case 'I': return mpt_cuint32(dest, src,  0, 0);
		
		case 'l': return mpt_cint64 (dest, src,  0, 0);
		case 'O': return mpt_cuint64(dest, src,  8, 0);
		case 'X': return mpt_cuint64(dest, src, 16, 0);
		case 'L': return mpt_cuint64(dest, src,  0, 0);
		
		case 'g':
		case 'f': return mpt_cfloat(dest, src, 0);
		case 'F':
		case 'G':
		case 'd': return mpt_cdouble(dest, src, 0);
		
		case 'D': return mpt_cldouble(dest, src, 0);
		
		default: errno = EINVAL; return -3;
	}
}
