/*!
 * text data output for file and terminal.
 */

#include <errno.h>
#include <stdio.h>
#include <ctype.h>

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
	char fmt[] = " %*lli";
	const char *fs = fmt + 1;
	int width = 1, adv = 'i', last = 0;
	
	if (!file) {
		errno = EFAULT; return -1;
	}
	while (1) {
		union {
			int64_t  l;
			int32_t  i;
			int16_t  h;
			int8_t   b;
			char     c;
			uint64_t L;
			uint32_t I;
			uint16_t H;
			uint8_t  B;
		} val;
		int	len = 0;
		
		/* update format settings */
		if (str && *str) {
			adv = mpt_outfmt_iget(*(str++), &width);
		}
		if (!adv || adv == 'i') {
			/* retry valid last successful conversion */
			if (islower(last) && (len = src->_vptr->conv(src, last, &val)) > 0) {
				switch (last) {
				  case 'i': val.l = val.i; break;
				  case 'h': val.l = val.h; break;
				  case 'b': val.l = val.b; break;
				  case 'c': val.l = val.c; break;
				  default:;
				}
			}
			else if ((len = src->_vptr->conv(src, last = 'i', &val.i)) > 0) val.l = val.i;
			else if ((len = src->_vptr->conv(src, last = 'l', &val.l)) > 0);
			else if ((len = src->_vptr->conv(src, last = 'h', &val.h)) > 0) val.l = val.h;
			else if ((len = src->_vptr->conv(src, last = 'b', &val.b)) > 0) val.l = val.b;
			else if ((len = src->_vptr->conv(src, last = 'c', &val.c)) > 0) val.l = val.c;
			else if (adv) break;
			else len = 0;
		}
		if (!len) {
			/* retry valid last successful conversion */
			if (isupper(last) && (len = src->_vptr->conv(src, last, &val)) > 0) {
				switch (last) {
				  case 'I': val.L = val.I; break;
				  case 'H': val.L = val.H; break;
				  case 'B': val.L = val.B; break;
				  case 'C': val.L = val.B; break;
				  default:;
				}
			}
			else if ((len = src->_vptr->conv(src, last = 'I', &val.I)) > 0) val.L = val.I;
			else if ((len = src->_vptr->conv(src, last = 'L', &val.L)) > 0);
			else if ((len = src->_vptr->conv(src, last = 'H', &val.H)) > 0) val.L = val.H;
			else if ((len = src->_vptr->conv(src, last = 'B', &val.B)) > 0) val.L = val.B;
			else if ((len = src->_vptr->conv(src, last = 'C', &val.B)) > 0) val.L = val.B;
			if (len <= 0) break;
		}
		if (len < 0) {
			fputs(" <badval>", file);
			continue;
		}
		if (!adv) continue;
		
		fmt[5] = adv;
		fprintf(file, fs, width, val.l);
		fs = fmt;
	}
	return 0;
}
