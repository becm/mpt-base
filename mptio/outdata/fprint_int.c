/*!
 * text data output for file and terminal.
 */

#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "output.h"

/*!
 * \ingroup mptMessage
 * \brief integer print decoding
 * 
 * Get format information for integer printing.
 * 
 * \param      fmt encoded format
 * \param[out] wp  field length
 * 
 * \return integer format
 */
extern int mpt_outfmt_iget(int fmt, int *wp)
{
	static const char types[] = "iuox";
	int width = fmt & 0x3f;
	
	if (!width) return 0;
	if (wp) *wp = width;
	
	return types[(fmt & 0xc0) >> 6];
}

/*!
 * \ingroup mptMessage
 * \brief print integer values
 * 
 * Print available integer elements to file.
 * 
 * \param file file descriptor
 * \param str  output format
 * \param src  floating point data source
 */
extern int mpt_fprint_int(FILE *file, const int8_t *str, MPT_INTERFACE(source) *src)
{
	static const char valsunsigned[] = "tuqy";
	char fmt[] = " %*lli";
	const char *fs = fmt + 1;
	int width = 1, adv = 'i', last = 0;
	
	if (!file) {
		errno = EFAULT;
		return -1;
	}
	while (1) {
		union {
			int64_t  x;
			int32_t  i;
			int16_t  n;
			int8_t   b;
			char     c;
			
			uint64_t t;
			uint32_t u;
			uint16_t q;
			uint8_t  y;
		} val;
		int len = 0;
		
		/* update format settings */
		if (str && *str) {
			adv = mpt_outfmt_iget(*(str++), &width);
		}
		if (!adv || adv == 'i') {
			/* retry valid last successful conversion */
			if (last && strchr(valsunsigned, last) && (len = src->_vptr->conv(src, last, &val)) > 0) {
				switch (last) {
				  case 'i': val.x = val.i; break;
				  case 'n': val.x = val.n; break;
				  case 'b': val.x = val.b; break;
				  case 'c': val.x = val.c; break;
				  default:;
				}
			}
			else if ((len = src->_vptr->conv(src, last = 'i', &val.i)) > 0) val.x = val.i;
			else if ((len = src->_vptr->conv(src, last = 'x', &val.x)) > 0);
			else if ((len = src->_vptr->conv(src, last = 'n', &val.n)) > 0) val.x = val.n;
			else if ((len = src->_vptr->conv(src, last = 'b', &val.b)) > 0) val.x = val.b;
			else if ((len = src->_vptr->conv(src, last = 'c', &val.c)) > 0) val.x = val.c;
			else if (adv) break;
			else len = 0;
		}
		if (!len) {
			/* retry valid last successful conversion */
			if (last && !strchr(valsunsigned, last) && (len = src->_vptr->conv(src, last, &val)) > 0) {
				switch (last) {
				  case 'u': val.t = val.u; break;
				  case 'q': val.t = val.q; break;
				  case 'y': val.t = val.y; break;
				  default:;
				}
			}
			else if ((len = src->_vptr->conv(src, last = 'u', &val.u)) > 0) val.t = val.u;
			else if ((len = src->_vptr->conv(src, last = 't', &val.t)) > 0);
			else if ((len = src->_vptr->conv(src, last = 'q', &val.q)) > 0) val.t = val.q;
			else if ((len = src->_vptr->conv(src, last = 'y', &val.y)) > 0) val.t = val.y;
			if (len <= 0) break;
		}
		if (len < 0) {
			fputs(" <badval>", file);
			continue;
		}
		if (!adv) continue;
		
		fmt[5] = adv;
		fprintf(file, fs, width, val.x);
		fs = fmt;
	}
	return 0;
}
