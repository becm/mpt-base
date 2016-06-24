/*!
 * text data output for file and terminal.
 */

#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "convert.h"
#include "meta.h"

#include "output.h"

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
extern int mpt_fprint_val(FILE *file, MPT_INTERFACE(metatype) *src)
{
	MPT_STRUCT(valfmt) fmt = MPT_VALFMT_INIT;
	static const char valfloat[] = "fde";
	static const char valunsigned[] = "tuqy";
	static const char intfmt[]  = "%*lli";
	static const char sintfmt[] = "%+*lli";
	static const char _intfmt[] = "% *lli";
	static const char uintfmt[]  = "%*llu";
	static const char suintfmt[] = "%+*llu";
	static const char _uintfmt[] = "%+*llu";
	static const char gfmt[]  = "%*g";
	static const char sgfmt[] = "%+*g";
	static const char _gfmt[] = "%*g";
	static const char gdecfmt[]  = "%*.*g";
	static const char sgdecfmt[] = "%+*.*g";
	static const char _gdecfmt[] = "% *.*g";
	static const char efmt[]  = "%*e";
	static const char sefmt[] = "%+*e";
	static const char _efmt[] = "% *e";
	static const char edecfmt[]  = "%*.*e";
	static const char sedecfmt[] = "%+*.*e";
	static const char _edecfmt[] = "%+*.*e";
	static const char ffmt[]  = "%*f";
	static const char sffmt[] = "%+*f";
	static const char _ffmt[] = "% *f";
	static const char fdecfmt[]  = "%*.*f";
	static const char sfdecfmt[] = "%+*.*f";
	static const char _fdecfmt[] = "%+*.*f";
	const char *currfmt = 0;
	int last = 0, nprint = 0;
	
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
			
			double   d;
			float    f;
		} val;
		int len;
		
		/* update format settings */
		(void) src->_vptr->conv(src, MPT_ENUM(TypeValFmt) | MPT_ENUM(ValueConsume), &fmt);
		
		/* retry valid last successful conversion */
		if (!last) {
			len = -1;
		}
		else if (last == 'c') {
			if ((len = src->_vptr->conv(src, last | MPT_ENUM(ValueConsume), &val.c)) > 0) {
				fprintf(file, " %*c", fmt.width, val.c);
				continue;
			}
		}
		else if (strchr(valfloat, last)) {
			if ((len = src->_vptr->conv(src, last | MPT_ENUM(ValueConsume), &val)) > 0) {
				switch (last) {
				  case 'f': val.d = val.f; break;
				  default:;
				}
			}
		}
		else if (strchr(valunsigned, last)) {
			if ((len = src->_vptr->conv(src, last | MPT_ENUM(ValueConsume), &val)) > 0) {
				switch (last) {
				  case 'u': val.t = val.u; break;
				  case 'q': val.t = val.q; break;
				  case 'y': val.t = val.y; break;
				  default:;
				}
			}
		}
		else if ((len = src->_vptr->conv(src, last | MPT_ENUM(ValueConsume), &val)) > 0) {
			switch (last) {
			  case 'i': val.x = val.i; break;
			  case 'n': val.x = val.i; break;
			  case 'b': val.x = val.i; break;
			  default:;
			}
		}
		if (!len) {
			return nprint;
		}
		/* (re-)try data query */
		if (len < 0) {
			if ((len = src->_vptr->conv(src, (last = 'c') | MPT_ENUM(ValueConsume), &val.c)) > 0) {
				fprintf(file, currfmt ? "%*c" : " %*c", fmt.width, val.c);
				continue;
			}
			else if ((len = src->_vptr->conv(src, (last = 'd') | MPT_ENUM(ValueConsume), &val.d)) > 0);
			else if ((len = src->_vptr->conv(src, (last = 'f') | MPT_ENUM(ValueConsume), &val.f)) > 0) val.d = val.f;
			else if ((len = src->_vptr->conv(src, (last = 'x') | MPT_ENUM(ValueConsume), &val.x)) > 0);
			else if ((len = src->_vptr->conv(src, (last = 'i') | MPT_ENUM(ValueConsume), &val.i)) > 0) val.x = val.i;
			else if ((len = src->_vptr->conv(src, (last = 'n') | MPT_ENUM(ValueConsume), &val.n)) > 0) val.x = val.n;
			else if ((len = src->_vptr->conv(src, (last = 'b') | MPT_ENUM(ValueConsume), &val.b)) > 0) val.x = val.b;
			else if ((len = src->_vptr->conv(src, (last = 't') | MPT_ENUM(ValueConsume), &val.t)) > 0);
			else if ((len = src->_vptr->conv(src, (last = 'u') | MPT_ENUM(ValueConsume), &val.n)) > 0) val.t = val.u;
			else if ((len = src->_vptr->conv(src, (last = 'q') | MPT_ENUM(ValueConsume), &val.n)) > 0) val.t = val.q;
			else if ((len = src->_vptr->conv(src, (last = 'y') | MPT_ENUM(ValueConsume), &val.y)) > 0) val.t = val.y;
			/* invalid data */
			else {
				return nprint;
			}
		}
		/* element separation */
		if (currfmt) {
			fputc(' ', file);
		}
		if (strchr(valfloat, last)) {
			switch (fmt.flt) {
			  default:
			  case  'f': currfmt = fmt.dec < 0 ?  ffmt :  fdecfmt; break;
			  case -'f': currfmt = fmt.dec < 0 ? sffmt : sfdecfmt; break;
			  case  'F': currfmt = fmt.dec < 0 ? _ffmt : _fdecfmt; break;
			  
			  case  'g': currfmt = fmt.dec < 0 ?  gfmt :  gdecfmt; break;
			  case -'g': currfmt = fmt.dec < 0 ? sgfmt : sgdecfmt; break;
			  case  'G': currfmt = fmt.dec < 0 ? _gfmt : _gdecfmt; break;
			  
			  case  'e': currfmt = fmt.dec < 0 ?  efmt :  edecfmt; break;
			  case -'e': currfmt = fmt.dec < 0 ? sefmt : sedecfmt; break;
			  case  'E': currfmt = fmt.dec < 0 ? _efmt : _edecfmt; break;
			}
			if (fmt.dec < 0) {
				fprintf(file, currfmt, fmt.width, val.d);
			} else {
				fprintf(file, currfmt, fmt.width, fmt.dec, val.d);
			}
			++nprint;
			continue;
		}
		else if (strchr(valunsigned, last)) {
			if (fmt.flt < 0) {
				currfmt = suintfmt;
			} else if (isupper(fmt.flt)) {
				currfmt = _uintfmt;
			} else {
				currfmt = uintfmt;
			}
		} else {
			if (fmt.flt < 0) {
				currfmt = sintfmt;
			} else if (isupper(fmt.flt)) {
				currfmt = _intfmt;
			} else {
				currfmt = intfmt;
			}
		}
		fprintf(file, currfmt, fmt.width, val.x);
		++nprint;
	}
	return nprint;
}
