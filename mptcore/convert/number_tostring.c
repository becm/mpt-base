/*!
 * print arguments to destination string.
 */

#include <stdio.h>

/* request format definitions */
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "types.h"

#include "convert.h"
/*!
 * \ingroup mptConvert
 * \brief print data element
 * 
 * Print single number to string.
 * 
 * \param  val  value with number data
 * \param  fmt  number format spec
 * \param  dest target string
 * \param  left max target size
 * 
 * \return consumed buffer size
 */
extern int mpt_number_tostring(const MPT_STRUCT(value) *val, MPT_STRUCT(value_format) fmt, char *dest, size_t left)
{
	const void *arg;
	uintptr_t type;
	int flg, sgn, len, dec;
	uint8_t wd;
	
	if (!(type = val->_type)) {
		return MPT_ERROR(BadType);
	}
	arg = val->_addr;
	
	flg = fmt.flags & 0xff;
	sgn = fmt.flags & MPT_VALFMT(Sign);
	wd  = fmt.width;
	dec = fmt.dec;
	
	if (fmt.flags & MPT_VALFMT(Left)) {
		wd = 0;
	}
	else if (wd > left) {
		return MPT_ERROR(MissingBuffer);
	}
	if (type == 'l') {
		type = mpt_type_int(sizeof(long));
	}
	switch (type) {
	/* single character */
	    case 'c':
		len = snprintf(dest, left, "%*c", wd, arg ? *((char*)arg) : 0);
		break;
	/* 8bit formats */
	    case 'b':
		if (!(flg & MPT_VALFMT(IntFlags))) {
			int8_t val = arg ? *((int8_t*)arg) : 0;
			if (sgn) {
				len = snprintf(dest, left, "%+*" PRIi8, wd, val);
			} else {
				len = snprintf(dest, left, "%*" PRIi8, wd, val);
			}
			break;
		}
		/* fall through */
	    case 'y':
		if (!(flg & MPT_VALFMT(IntFlags))) {
			uint8_t val = arg ? *((uint8_t*)arg) : 0;
			len = snprintf(dest, left, "%*" PRIu8, wd, val);
			break;
		}
		if (flg & MPT_VALFMT(IntHex)) {
			uint8_t val = arg ? *((uint8_t*)arg) : 0;
			len = snprintf(dest, left, "%*" PRIx8, wd, val);
			break;
		}
		if (flg & MPT_VALFMT(IntOctal)) {
			uint8_t val = arg ? *((uint8_t*)arg) : 0;
			len = snprintf(dest, left, "%*" PRIo8, wd, val);
			break;
		}
		return MPT_ERROR(BadValue);
	/* 16bit formats */
	    case 'n':
		if (!(flg & MPT_VALFMT(IntFlags))) {
			int16_t val = arg ? *((int16_t*)arg) : 0;
			if (sgn) {
				len = snprintf(dest, left, "%+*" PRIi16, wd, val);
			} else {
				len = snprintf(dest, left, "%*" PRIi16, wd, val);
			}
			break;
		}
		/* fall through */
	    case 'q':
		if (!(flg & MPT_VALFMT(IntFlags))) {
			uint16_t val = arg ? *((uint16_t*)arg) : 0;
			len = snprintf(dest, left, "%*" PRIu16, wd, val);
			break;
		}
		if (flg & MPT_VALFMT(IntHex)) {
			uint16_t val = arg ? *((uint16_t*)arg) : 0;
			len = snprintf(dest, left, "%*" PRIx16, wd, val);
			break;
		}
		if (flg & MPT_VALFMT(IntOctal)) {
			uint16_t val = arg ? *((uint16_t*)arg) : 0;
			len = snprintf(dest, left, "%*" PRIo16, wd, val);
			break;
		}
		return MPT_ERROR(BadValue);
	/* 32bit formats */
	    case 'i':
		if (!(flg & MPT_VALFMT(IntFlags))) {
			int32_t val = arg ? *((int32_t*)arg) : 0;
			if (sgn) {
				len = snprintf(dest, left, "%+*" PRIi32, wd, val);
			} else {
				len = snprintf(dest, left, "%*" PRIi32, wd, val);
			}
			break;
		}
		/* fall through */
	    case 'u':
		if (!(flg & MPT_VALFMT(IntFlags))) {
			uint32_t val = arg ? *((uint32_t*)arg) : 0;
			len = snprintf(dest, left, "%*" PRIu32, wd, val);
			break;
		}
		if (flg & MPT_VALFMT(IntHex)) {
			uint32_t val = arg ? *((uint32_t*)arg) : 0;
			len = snprintf(dest, left, "%*" PRIx32, wd, val);
			break;
		}
		if (flg & MPT_VALFMT(IntOctal)) {
			uint32_t val = arg ? *((uint32_t*)arg) : 0;
			len = snprintf(dest, left, "%*" PRIo32, wd, val);
			break;
		}
		return MPT_ERROR(BadValue);
	/* 64bit formats */
	    case 'x':
		if (!(flg & MPT_VALFMT(IntFlags))) {
			int64_t val = arg ? *((int64_t*)arg) : 0;
			if (sgn) {
				len = snprintf(dest, left, "%+*" PRIi64, wd, val);
			} else {
				len = snprintf(dest, left, "%*" PRIi64, wd, val);
			}
			break;
		}
		/* fall through */
	    case 't':
		if (!(flg & MPT_VALFMT(IntFlags))) {
			uint64_t val = arg ? *((uint64_t*)arg) : 0;
			len = snprintf(dest, left, "%*" PRIu64, wd, val);
			break;
		}
		if (flg & MPT_VALFMT(IntHex)) {
			uint64_t val = arg ? *((uint64_t*)arg) : 0;
			len = snprintf(dest, left, "%*" PRIx64, wd, val);
			break;
		}
		if (flg & MPT_VALFMT(IntOctal)) {
			uint64_t val = arg ? *((uint64_t*)arg) : 0;
			len = snprintf(dest, left, "%*" PRIo64, wd, val);
			break;
		}
		return MPT_ERROR(BadValue);
	/* floating point formats */
	    case 'f':
		if (!dec) {
			float val = arg ? *((float*)arg) : 0;
			if (sgn) {
				len = snprintf(dest, left, "%+*g", wd, val);
			} else {
				len = snprintf(dest, left, "%*g", wd, val);
			}
			break;
		}
		if (!(flg & MPT_VALFMT(FltFlags))) {
			float val = arg ? *((float*)arg) : 0;
			if (sgn) {
				len = snprintf(dest, left, "%+*.*f", wd, dec, val);
			} else {
				len = snprintf(dest, left, "%*.*f", wd, dec, val);
			}
			break;
		}
#if __STDC_VERSION__ >= 199901L
		if (flg & MPT_VALFMT(FltHex)) {
			float val = arg ? *((float*)arg) : 0;
			if (sgn) {
				len = snprintf(dest, left, "%*.*a", wd, dec, val);
			} else {
				len = snprintf(dest, left, "%*.*a", wd, dec, val);
			}
			break;
		}
#endif
		if (flg & MPT_VALFMT(Scientific)) {
			float val = arg ? *((float*)arg) : 0;
			if (sgn) {
				len = snprintf(dest, left, "%+*.*e", wd, dec, val);
			} else {
				len = snprintf(dest, left, "%*.*e", wd, dec, val);
			}
			break;
		}
		return MPT_ERROR(BadValue);
	    case 'd':
		if (!dec) {
			double val = arg ? *((double*)arg) : 0;
			if (sgn) {
				len = snprintf(dest, left, "%+*g", wd, val);
			} else {
				len = snprintf(dest, left, "%*g", wd, val);
			}
			break;
		}
		if (!(flg & MPT_VALFMT(FltFlags))) {
			double val = arg ? *((double*)arg) : 0;
			if (sgn) {
				len = snprintf(dest, left, "%+*.*f", wd, dec, val);
			} else {
				len = snprintf(dest, left, "%*.*f", wd, dec, val);
			}
			break;
		}
#if __STDC_VERSION__ >= 199901L
		if (flg & MPT_VALFMT(FltHex)) {
			double val = arg ? *((double*)arg) : 0;
			if (sgn) {
				len = snprintf(dest, left, "%+*.*a", wd, dec, val);
			} else {
				len = snprintf(dest, left, "%*.*a", wd, dec, val);
			}
			break;
		}
#endif
		if (flg & MPT_VALFMT(Scientific)) {
			double val = arg ? *((double*)arg) : 0;
			if (sgn) {
				len = snprintf(dest, left, "%+*.*e", wd, dec, val);
			} else {
				len = snprintf(dest, left, "%*.*e", wd, dec, val);
			}
			break;
		}
		return MPT_ERROR(BadValue);
#ifdef _MPT_FLOAT_EXTENDED_H
	    case 'e':
		if (!dec) {
			long double val = arg ? *((long double*)arg) : 0;
			if (sgn) {
				len = snprintf(dest, left, "%+*Lg", wd, val);
			} else {
				len = snprintf(dest, left, "%*Lg", wd, val);
			}
			break;
		}
		if (!(flg & MPT_VALFMT(FltFlags))) {
			long double val = arg ? *((long double*)arg) : 0;
			if (sgn) {
				len = snprintf(dest, left, "%+*.*Lf", wd, dec, val);
			} else {
				len = snprintf(dest, left, "%*.*Lf", wd, dec, val);
			}
			break;
		}
# if __STDC_VERSION__ >= 199901L
		if (flg & MPT_VALFMT(FltHex)) {
			long double val = arg ? *((long double*)arg) : 0;
			if (sgn) {
				len = snprintf(dest, left, "%+*.*LA", wd, dec, val);
			} else {
				len = snprintf(dest, left, "%*.*LA", wd, dec, val);
			}
			break;
		}
# endif
		if (flg & MPT_VALFMT(Scientific)) {
			long double val = arg ? *((long double*)arg) : 0;
			if (sgn) {
				len = snprintf(dest, left, "%+*.*Le", wd, dec, val);
			} else {
				len = snprintf(dest, left, "%*.*Le", wd, dec, val);
			}
			break;
		}
		return MPT_ERROR(BadValue);
#endif /* _MPT_FLOAT_EXTENDED_H */
	    default:
		return MPT_ERROR(BadType);
	}
	if (len < 0) {
		return len;
	}
	if ((size_t) len >= left) {
		return MPT_ERROR(MissingBuffer);
	}
	if (flg & MPT_VALFMT(Left)) {
		wd = fmt.width;
		if (left < (size_t) wd) {
			return MPT_ERROR(MissingBuffer);
		}
		wd -= len;
		while (wd-- > 0) {
			dest[len++] = ' ';
		}
		if ((size_t) len < left) {
			dest[len] = 0;
		}
	}
	return len;
}

