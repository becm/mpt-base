/*!
 * print arguments to destination string.
 */

#include <string.h>
#include <stdio.h>

/* request format definitions */
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "layout.h"
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
extern int mpt_tostring(const void *data, const char *fmt, int (*save)(void *, const char *, size_t ), void *dest)
{
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
		else if ((adv = mpt_data_print(buf, sizeof(buf), *fmt, &data)) < 0) {
			return -1;
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
extern int mpt_data_print(char *dest, size_t left, int type, const void **ptr)
{
	char * const *arg = *ptr;
	size_t size;
	int len;
	
	switch ( type ) {
	    case '#':
		size = 4 * sizeof(uint8_t);
		if ( ((char *)arg)[0] != -1 ) {
			MPT_STRUCT(color) *c = (void *) arg;
			len = snprintf(dest, left, "#%02x%02x%02x%02x", c->red, c->green, c->blue, c->alpha);
		} else {
			MPT_STRUCT(color) *c = (void *) arg;
			len = snprintf(dest, left, "#%02x%02x%02x", c->red, c->green, c->blue);
		}
		break;
	    case 'c':
		size = sizeof(char);
		len = snprintf(dest, left, "%c", *((char*)arg));
		break;
	    case 'C':
		size = sizeof(unsigned char);
		len = snprintf(dest, left, "%u", *((unsigned char*)arg));
		break;
	    case 'b':
		size = sizeof(int8_t);
		len = snprintf(dest, left, "%"PRIi8, *((int8_t*)arg));
		break;
	    case 'B':
		size = sizeof(uint8_t);
		len = snprintf(dest, left, "%"PRIu8, *((uint8_t*)arg));
		break;
	    case 'h':
		size = sizeof(int16_t);
	    case 'H':
		size = sizeof(uint16_t);
		len = snprintf(dest, left, "%"PRIu16, *((uint16_t*)arg));
		break;
	    case 'i':
		size = sizeof(int32_t);
		len = snprintf(dest, left, "%"PRIi32, *((int32_t*)arg));
		break;
	    case 'I':
		size = sizeof(uint32_t);
		len = snprintf(dest, left, "%"PRIu32, *((uint32_t*)arg));
		break;
	    case 'o':
		size = sizeof(uint32_t);
		len = snprintf(dest, left, "0%"PRIo32, *((uint32_t*)arg));
		break;
	    case 'x':
		size = sizeof(uint32_t);
		len = snprintf(dest, left, "0x%"PRIx32, *((uint32_t*)arg));
		break;
	    case 'l':
		size = sizeof(int64_t);
		len = snprintf(dest, left, "%"PRIu64, *((int64_t*)arg));
		break;
	    case 'L':
		size = sizeof(uint64_t);
		len = snprintf(dest, left, "%"PRIu64, *((uint64_t*)arg));
		break;
	    case 'O':
		size = sizeof(uint64_t);
		len = snprintf(dest, left, "0%"PRIo64, *((uint64_t*)arg));
		break;
	    case 'X':
		size = sizeof(uint64_t);
		len = snprintf(dest, left, "0x%"PRIx64, *((uint64_t*)arg));
		break;
	    case 'g':
		size = sizeof(float);
		len = snprintf(dest, left, "%g", *((float*)arg));
		break;
	    case 'f':
		size = sizeof(float);
		len = snprintf(dest, left, "%f", *((float*)arg));
		break;
	    case 'e':
		size = sizeof(float);
		len = snprintf(dest, left, "%e", *((float*)arg));
		break;
	    case 'a':
		size = sizeof(float);
		len = snprintf(dest, left, "%a", *((float*)arg));
		break;
	    case 'G':
		size = sizeof(double);
		len = snprintf(dest, left, "%g", *((double*)arg));
		break;
	    case 'd':
		size = sizeof(double);
		len = snprintf(dest, left, "%f", *((double*)arg));
		break;
	    case 'E':
		size = sizeof(double);
		len = snprintf(dest, left, "%E", *((double*)arg));
		break;
	    case 'A':
		size = sizeof(double);
		len = snprintf(dest, left, "%A", *((double*)arg));
		break;
	    case 's':
		size = sizeof(char*);
		len = *arg ? snprintf(dest, left, "%s", *arg) : 0;
		break;
	    case 'S':
		size = sizeof(char**);
		len = (*arg && **((char ***)arg)) ? snprintf(dest, left, "%s", **((char ***)arg)) : 0;
		break;
	    default:
		return -1;
	}
	if (len < 0) return len;
	if ((size_t) len >= left) return -2;
	*ptr = ((char *)arg) + size;
	return len;
}

