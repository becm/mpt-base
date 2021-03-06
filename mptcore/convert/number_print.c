/*!
 * print arguments to destination string.
 */

#include <stdio.h>

/* request format definitions */
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "convert.h"
/*!
 * \ingroup mptConvert
 * \brief print data element
 * 
 * Print single data element to string.
 * 
 * \param  dest target string
 * \param  left max target size
 * \param  type data type
 * \param  ptr  address of current data element
 * 
 * \return consumed buffer size
 */
extern int mpt_number_print(char *dest, size_t left, MPT_STRUCT(value_format) fmt, int type, const void *arg)
{
	int flg, sgn, len, dec;
	uint8_t wd;
	
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
		len = snprintf(dest, left, "%*c", wd, *((char*)arg));
		break;
	/* 8bit formats */
	    case 'b':
		if (!(flg & MPT_VALFMT(IntFlags))) {
			if (sgn) {
				len = snprintf(dest, left, "%+*" PRIi8, wd, *((int8_t*)arg));
			} else {
				len = snprintf(dest, left, "%*" PRIi8, wd, *((int8_t*)arg));
			}
			break;
		}
		/* fall through */
	    case 'y':
		if (!(flg & MPT_VALFMT(IntFlags))) {
			len = snprintf(dest, left, "%*" PRIu8, wd, *((uint8_t*)arg));
			break;
		}
		if (flg & MPT_VALFMT(IntHex)) {
			len = snprintf(dest, left, "%*" PRIx8, wd, *((uint8_t*)arg));
			break;
		}
		if (flg & MPT_VALFMT(IntOctal)) {
			len = snprintf(dest, left, "%*" PRIo8, wd, *((uint8_t*)arg));
			break;
		}
		return MPT_ERROR(BadValue);
	/* 16bit formats */
	    case 'n':
		if (!(flg & MPT_VALFMT(IntFlags))) {
			if (sgn) {
				len = snprintf(dest, left, "%+*" PRIi16, wd, *((int16_t*)arg));
			} else {
				len = snprintf(dest, left, "%*" PRIi16, wd, *((int16_t*)arg));
			}
			break;
		}
		/* fall through */
	    case 'q':
		if (!(flg & MPT_VALFMT(IntFlags))) {
			len = snprintf(dest, left, "%*" PRIu16, wd, *((uint16_t*)arg));
			break;
		}
		if (flg & MPT_VALFMT(IntHex)) {
			len = snprintf(dest, left, "%*" PRIx16, wd, *((uint16_t*)arg));
			break;
		}
		if (flg & MPT_VALFMT(IntOctal)) {
			len = snprintf(dest, left, "%*" PRIo16, wd, *((uint16_t*)arg));
			break;
		}
		return MPT_ERROR(BadValue);
	/* 32bit formats */
	    case 'i':
		if (!(flg & MPT_VALFMT(IntFlags))) {
			if (sgn) {
				len = snprintf(dest, left, "%+*" PRIi32, wd, *((int32_t*)arg));
			} else {
				len = snprintf(dest, left, "%*" PRIi32, wd, *((int32_t*)arg));
			}
			break;
		}
		/* fall through */
	    case 'u':
		if (!(flg & MPT_VALFMT(IntFlags))) {
			len = snprintf(dest, left, "%*" PRIu32, wd, *((uint32_t*)arg));
			break;
		}
		if (flg & MPT_VALFMT(IntHex)) {
			len = snprintf(dest, left, "%*" PRIx32, wd, *((uint32_t*)arg));
			break;
		}
		if (flg & MPT_VALFMT(IntOctal)) {
			len = snprintf(dest, left, "%*" PRIo32, wd, *((uint32_t*)arg));
			break;
		}
		return MPT_ERROR(BadValue);
	/* 64bit formats */
	    case 'x':
		if (!(flg & MPT_VALFMT(IntFlags))) {
			if (sgn) {
				len = snprintf(dest, left, "%+*" PRIi64, wd, *((int64_t*)arg));
			} else {
				len = snprintf(dest, left, "%*" PRIi64, wd, *((int64_t*)arg));
			}
			break;
		}
		/* fall through */
	    case 't':
		if (!(flg & MPT_VALFMT(IntFlags))) {
			len = snprintf(dest, left, "%*" PRIu64, wd, *((uint64_t*)arg));
			break;
		}
		if (flg & MPT_VALFMT(IntHex)) {
			len = snprintf(dest, left, "%*" PRIx64, wd, *((uint64_t*)arg));
			break;
		}
		if (flg & MPT_VALFMT(IntOctal)) {
			len = snprintf(dest, left, "%*" PRIo64, wd, *((uint64_t*)arg));
			break;
		}
		return MPT_ERROR(BadValue);
	/* floating point formats */
	    case 'f':
		if (!dec) {
			if (sgn) {
				len = snprintf(dest, left, "%+*g", wd, *((float*)arg));
			} else {
				len = snprintf(dest, left, "%*g", wd, *((float*)arg));
			}
			break;
		}
		if (!(flg & MPT_VALFMT(FltFlags))) {
			if (sgn) {
				len = snprintf(dest, left, "%+*.*f", wd, dec, *((float*)arg));
			} else {
				len = snprintf(dest, left, "%*.*f", wd, dec, *((float*)arg));
			}
			break;
		}
#if __STDC_VERSION__ >= 199901L
		if (flg & MPT_VALFMT(FltHex)) {
			if (sgn) {
				len = snprintf(dest, left, "%*.*a", wd, dec, *((float*)arg));
			} else {
				len = snprintf(dest, left, "%*.*a", wd, dec, *((float*)arg));
			}
			break;
		}
#endif
		if (flg & MPT_VALFMT(Scientific)) {
			if (sgn) {
				len = snprintf(dest, left, "%+*.*e", wd, dec, *((float*)arg));
			} else {
				len = snprintf(dest, left, "%*.*e", wd, dec, *((float*)arg));
			}
			break;
		}
		return MPT_ERROR(BadValue);
	    case 'd':
		if (!dec) {
			if (sgn) {
				len = snprintf(dest, left, "%+*g", wd, *((double*)arg));
			} else {
				len = snprintf(dest, left, "%*g", wd, *((double*)arg));
			}
			break;
		}
		if (!(flg & MPT_VALFMT(FltFlags))) {
			if (sgn) {
				len = snprintf(dest, left, "%+*.*f", wd, dec, *((double*)arg));
			} else {
				len = snprintf(dest, left, "%*.*f", wd, dec, *((double*)arg));
			}
			break;
		}
#if __STDC_VERSION__ >= 199901L
		if (flg & MPT_VALFMT(FltHex)) {
			if (sgn) {
				len = snprintf(dest, left, "%+*.*a", wd, dec, *((double*)arg));
			} else {
				len = snprintf(dest, left, "%*.*a", wd, dec, *((double*)arg));
			}
			break;
		}
#endif
		if (flg & MPT_VALFMT(Scientific)) {
			if (sgn) {
				len = snprintf(dest, left, "%+*.*e", wd, dec, *((double*)arg));
			} else {
				len = snprintf(dest, left, "%*.*e", wd, dec, *((double*)arg));
			}
			break;
		}
		return MPT_ERROR(BadValue);
#ifdef _MPT_FLOAT_EXTENDED_H
	    case 'e':
		if (!dec) {
			if (sgn) {
				len = snprintf(dest, left, "%+*Lg", wd, *((long double*)arg));
			} else {
				len = snprintf(dest, left, "%*Lg", wd, *((long double*)arg));
			}
			break;
		}
		if (!(flg & MPT_VALFMT(FltFlags))) {
			if (sgn) {
				len = snprintf(dest, left, "%+*.*Lf", wd, dec, *((long double*)arg));
			} else {
				len = snprintf(dest, left, "%*.*Lf", wd, dec, *((long double*)arg));
			}
			break;
		}
# if __STDC_VERSION__ >= 199901L
		if (flg & MPT_VALFMT(FltHex)) {
			if (sgn) {
				len = snprintf(dest, left, "%+*.*LA", wd, dec, *((long double*)arg));
			} else {
				len = snprintf(dest, left, "%*.*LA", wd, dec, *((long double*)arg));
			}
			break;
		}
# endif
		if (flg & MPT_VALFMT(Scientific)) {
			if (sgn) {
				len = snprintf(dest, left, "%+*.*Le", wd, dec, *((long double*)arg));
			} else {
				len = snprintf(dest, left, "%*.*Le", wd, dec, *((long double*)arg));
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
		dest += len;
		wd = fmt.width;
		if (left <= (size_t) wd) {
			return MPT_ERROR(MissingBuffer);
		}
		wd -= len;
		while (wd-- > 0) {
			*(dest++) = ' ';
		}
		*(dest++) = 0;
	}
	return len;
}

