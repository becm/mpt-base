/*!
 * print arguments to destination string.
 */

#include <string.h>
#include <stdio.h>

/* request format definitions */
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "convert.h"

#include "../mptplot/layout.h"

/*!
 * \ingroup mptConvert
 * \brief print data
 * 
 * Add stringifyed data to buffer.
 * 
 * \param data raw data base address
 * \param fmt  format for data
 * \param save function to save text data
 * \param dest save target pointer
 * 
 * \return number of processed elements (0 = all)
 */
extern int mpt_tostring(const MPT_STRUCT(value) *val, ssize_t (*save)(void *, const char *, size_t), void *dest)
{
	const char *fmt = val->fmt;
	const void *data = val->ptr;
	int cont = 0;
	
	if (!fmt) {
		if (data && save(dest, data, strlen(data)) < 0) {
			return -2;
		}
		return 0;
	}
	while (*fmt) {
		char buf[256];
		const char *txt = buf;
		int adv;
		
		if (*fmt == 's' || *fmt == 'k') {
			const char * const * ptr = data;
			adv = (txt = *ptr) ? strlen(txt) : 0;
			data = ptr + 1;
		}
		else if ((adv = mpt_data_print(buf, sizeof(buf), *fmt, data)) < 0) {
			return -1;
		}
		else {
			int len;
			if ((len = mpt_valsize(*fmt)) < 0) {
				return -1;
			}
			if (!len) len = sizeof(void *);
			
			data = ((uint8_t *) data) + len;
		}
		if (cont && save(dest, " ", 1) < 1) return -2;
		if (save(dest, txt, adv) < adv) return cont ? cont : -2;
		++fmt;
		++cont;
	}
	return 0;
}

/*!
 * \ingroup mptConvert
 * \brief print data element
 * 
 * Print single data element to string.
 * 
 * \param      dest target string
 * \param      left max target size
 * \param      type data type
 * \param[in]  ptr  address of current data element
 * \param[out] ptr  address of next data element
 * 
 * \return consumed buffer size
 */
extern int mpt_data_print(char *dest, size_t left, int type, const void *arg)
{
	int len;
	
	if (type == 'l') {
		type = MPT_ENUM(TypeLong);
	}
	switch (type) {
	/* color format */
	    case MPT_ENUM(TypeColor):
		if (((const MPT_STRUCT(color) *)arg)->alpha != 0xff) {
			const MPT_STRUCT(color) *c = arg;
			len = snprintf(dest, left, "#%02x%02x%02x%02x", c->red, c->green, c->blue, c->alpha);
		} else {
			MPT_STRUCT(color) *c = (void *) arg;
			len = snprintf(dest, left, "#%02x%02x%02x", c->red, c->green, c->blue);
		}
		break;
	/* single character */
	    case 'c':
		len = snprintf(dest, left, "%c", *((char*)arg));
		break;
	/* 8bit formats */
	    case 'b':
		len = snprintf(dest, left, "%"PRIi8, *((int8_t*)arg));
		break;
	    case 'y':
		len = snprintf(dest, left, "%"PRIu8, *((uint8_t*)arg));
		break;
	    case 'y' | MPT_ENUM(PrintNumberHex):
		len = snprintf(dest, left, "%"PRIx8, *((uint8_t*)arg));
		break;
	    case 'y' | MPT_ENUM(PrintIntOctal):
		len = snprintf(dest, left, "%"PRIo8, *((uint8_t*)arg));
		break;
	/* 16bit formats */
	    case 'n':
		len = snprintf(dest, left, "%"PRIi16, *((int16_t*)arg));
		break;
	    case 'q':
		len = snprintf(dest, left, "%"PRIu16, *((uint16_t*)arg));
		break;
	    case 'q' | MPT_ENUM(PrintNumberHex):
		len = snprintf(dest, left, "%"PRIx16, *((uint16_t*)arg));
		break;
	    case 'q' | MPT_ENUM(PrintIntOctal):
		len = snprintf(dest, left, "%"PRIo16, *((uint16_t*)arg));
		break;
	/* 32bit formats */
	    case 'i':
		len = snprintf(dest, left, "%"PRIi32, *((int32_t*)arg));
		break;
	    case 'u':
		len = snprintf(dest, left, "%"PRIu32, *((uint32_t*)arg));
		break;
	    case 'u' | MPT_ENUM(PrintNumberHex):
		len = snprintf(dest, left, "%"PRIx32, *((uint32_t*)arg));
		break;
	    case 'u' | MPT_ENUM(PrintIntOctal):
		len = snprintf(dest, left, "%"PRIo32, *((uint32_t*)arg));
		break;
	/* 64bit formats */
	    case 'x':
		len = snprintf(dest, left, "%"PRIi64, *((int64_t*)arg));
		break;
	    case 't':
		len = snprintf(dest, left, "%"PRIu64, *((uint64_t*)arg));
		break;
	    case 't' | MPT_ENUM(PrintNumberHex):
		len = snprintf(dest, left, "%"PRIx64, *((uint64_t*)arg));
		break;
	    case 't' | MPT_ENUM(PrintIntOctal):
		len = snprintf(dest, left, "%"PRIo64, *((uint64_t*)arg));
		break;
	/* floating point formats */
	    case 'f':
		len = snprintf(dest, left, "%g", *((float*)arg));
		break;
	    case 'f' | MPT_ENUM(PrintScientific):
		len = snprintf(dest, left, "%e", *((float*)arg));
		break;
#if __STDC_VERSION__ >= 199901L
	    case 'f' | MPT_ENUM(PrintNumberHex):
		len = snprintf(dest, left, "%a", *((float*)arg));
		break;
#endif
	    case 'd':
		len = snprintf(dest, left, "%g", *((double*)arg));
		break;
	    case 'd' | MPT_ENUM(PrintScientific):
		len = snprintf(dest, left, "%e", *((double*)arg));
		break;
#if __STDC_VERSION__ >= 199901L
	    case 'd' | MPT_ENUM(PrintNumberHex):
		len = snprintf(dest, left, "%a", *((double*)arg));
		break;
#endif
#ifdef _MPT_FLOAT_EXTENDED_H
	    case 'e':
		len = snprintf(dest, left, "%Lg", *((long double*)arg));
		break;
	    case 'e' | MPT_ENUM(PrintScientific):
		len = snprintf(dest, left, "%Le", *((long double*)arg));
		break;
# if __STDC_VERSION__ >= 199901L
	    case 'e' | MPT_ENUM(PrintNumberHex):
		len = snprintf(dest, left, "%LA", *((long double*)arg));
		break;
# endif
#endif /* _MPT_FLOAT_EXTENDED_H */
	/* string pointers */
	    case 's':
		len = (*(char **)arg) ? snprintf(dest, left, "%s", *(char **)arg) : 0;
		break;
	    default:
		return -1;
	}
	if (len < 0) return len;
	if ((size_t) len >= left) return -2;
	return len;
}

