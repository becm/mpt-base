/*!
 * print arguments to destination string.
 */

#include <string.h>
#include <stdio.h>

/* request format definitions */
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "plot.h"
#include "convert.h"


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
extern int mpt_tostring(const MPT_STRUCT(value) *val, int (*save)(void *, const char *, size_t ), void *dest)
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
	
	switch (type) {
	    case '#':
		if (((const MPT_STRUCT(color) *)arg)->alpha != 0xff) {
			const MPT_STRUCT(color) *c = arg;
			len = snprintf(dest, left, "#%02x%02x%02x%02x", c->red, c->green, c->blue, c->alpha);
		} else {
			MPT_STRUCT(color) *c = (void *) arg;
			len = snprintf(dest, left, "#%02x%02x%02x", c->red, c->green, c->blue);
		}
		break;
	    case 'c': case 'C':
		len = snprintf(dest, left, "%c", *((char*)arg));
		break;
		
	/* 8bit formats */
	    case 'b':
		len = snprintf(dest, left, "%"PRIi8, *((int8_t*)arg));
		break;
	    case 'B':
		len = snprintf(dest, left, "%"PRIu8, *((uint8_t*)arg));
		break;
	    case 'y':
		len = snprintf(dest, left, "%"PRIx8, *((uint8_t*)arg));
		break;
	    case 'Y':
		len = snprintf(dest, left, "%"PRIo8, *((uint8_t*)arg));
		break;
		
	/* 16bit formats */
	    case 'h':
		len = snprintf(dest, left, "%"PRIi16, *((int16_t*)arg));
		break;
	    case 'H':
		len = snprintf(dest, left, "%"PRIu16, *((uint16_t*)arg));
		break;
	    case 'n':
		len = snprintf(dest, left, "%"PRIx16, *((uint16_t*)arg));
		break;
	    case 'N':
		len = snprintf(dest, left, "%"PRIo16, *((uint16_t*)arg));
		break;
		
	/* 32bit formats */
	    case 'i':
		len = snprintf(dest, left, "%"PRIi32, *((int32_t*)arg));
		break;
	    case 'I':
		len = snprintf(dest, left, "%"PRIu32, *((uint32_t*)arg));
		break;
	    case 'u':
		len = snprintf(dest, left, "%"PRIx32, *((uint32_t*)arg));
		break;
	    case 'U':
		len = snprintf(dest, left, "%"PRIo32, *((uint32_t*)arg));
		break;
		
	/* 64bit formats */
	    case 'l':
		len = snprintf(dest, left, "%"PRIi64, *((int64_t*)arg));
		break;
	    case 'L':
		len = snprintf(dest, left, "%"PRIu64, *((uint64_t*)arg));
		break;
	    case 'x':
		len = snprintf(dest, left, "%"PRIx64, *((uint64_t*)arg));
		break;
	    case 'X':
		len = snprintf(dest, left, "%"PRIo64, *((uint64_t*)arg));
		break;
		
	/* floating point formats */
	    case 'F':
		len = snprintf(dest, left, "%g", *((float*)arg));
		break;
	    case 'f':
		len = snprintf(dest, left, "%e", *((float*)arg));
		break;
	    case 'D':
		len = snprintf(dest, left, "%g", *((double*)arg));
		break;
	    case 'd':
		len = snprintf(dest, left, "%e", *((double*)arg));
		break;
	    case 'E':
		len = snprintf(dest, left, "%g", (double) *((long double*)arg));
		break;
	    case 'e':
		len = snprintf(dest, left, "%e", (double) *((long double*)arg));
		break;
		
	/* string pointers */
	    case 's':
		len = (*(char **)arg) ? snprintf(dest, left, "%s", *(char **)arg) : 0;
		break;
	    case 'S':
		len = (*(char **)arg && **((char ***)arg)) ? snprintf(dest, left, "%s", **((char ***)arg)) : 0;
		break;
	    default:
		return -1;
	}
	if (len < 0) return len;
	if ((size_t) len >= left) return -2;
	return len;
}

