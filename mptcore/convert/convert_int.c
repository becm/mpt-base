/*!
 * get signed integer value from string
 */

#include <ctype.h>
#include <stdlib.h>
#include <inttypes.h>
#include <limits.h>

#include "convert.h"

#define GET_STRING_FCN(get, name, type) \
int name(type *val, const char *src, int base, const type range[2]) \
{ \
	type tmp; \
	int ret; \
	\
	if ((ret = get(&tmp, sizeof(type), src, base)) <= 0) { \
		return ret; \
	} \
	if (range && (tmp < range[0] || tmp > range[1])) { \
		return MPT_ERROR(BadValue); \
	} \
	if (val) *val = tmp; \
	return ret; \
}

GET_STRING_FCN(_mpt_convert_int, mpt_cint8,  int8_t )
GET_STRING_FCN(_mpt_convert_int, mpt_cint16, int16_t)
GET_STRING_FCN(_mpt_convert_int, mpt_cint32, int32_t)
GET_STRING_FCN(_mpt_convert_int, mpt_cint64, int64_t)

GET_STRING_FCN(_mpt_convert_int, mpt_cchar,  char)
GET_STRING_FCN(_mpt_convert_int, mpt_cint,   int )
GET_STRING_FCN(_mpt_convert_int, mpt_clong,  long)

extern int _mpt_convert_int(void *val, size_t vlen, const char *src, int base)
{
	intmax_t tmp;
	char *end;
	
	if (!(end = (char *) src)) {
		return MPT_ERROR(BadArgument);
	}
	if (!*src) {
		return 0;
	}
	/* max size unsigend integer */
	tmp = strtoimax(src, &end, base);
	if (end == src) {
		/* accept space as empty string */
		while (*src) {
			if (!isspace(*src++)) {
				return MPT_ERROR(BadType);
			}
		}
		return 0;
	}
	switch (vlen) {
	  case sizeof(int8_t) :
		if (tmp < INT8_MIN || tmp > INT8_MAX) {
			return MPT_ERROR(BadValue);
		}
		if (val) *((int8_t *) val) = tmp;
		break;
	  case sizeof(int16_t):
		if (tmp < INT16_MIN || tmp > INT16_MAX) {
			return MPT_ERROR(BadValue);
		}
		if (val) *((int16_t *) val) = tmp;
		break;
	  case sizeof(int32_t):
		if (tmp < INT32_MIN || tmp > INT32_MAX) {
			return MPT_ERROR(BadValue);
		}
		if (val) *((int32_t *) val) = tmp;
		break;
	  case sizeof(int64_t):
		if (tmp < INT64_MIN || tmp > INT64_MAX) {
			return MPT_ERROR(BadValue);
		}
		if (val) *((int64_t *) val) = tmp;
		break;
	  default:
		return MPT_ERROR(BadType);
	}
	
	return end - src;
}

GET_STRING_FCN(_mpt_convert_uint, mpt_cuint8,  uint8_t )
GET_STRING_FCN(_mpt_convert_uint, mpt_cuint16, uint16_t)
GET_STRING_FCN(_mpt_convert_uint, mpt_cuint32, uint32_t)
GET_STRING_FCN(_mpt_convert_uint, mpt_cuint64, uint64_t)

GET_STRING_FCN(_mpt_convert_uint, mpt_cuchar,  unsigned char)
GET_STRING_FCN(_mpt_convert_uint, mpt_cuint,   unsigned int )
GET_STRING_FCN(_mpt_convert_uint, mpt_culong,  unsigned long)

extern int _mpt_convert_uint(void *val, size_t vlen, const char *src, int base)
{
	uintmax_t tmp;
	char *end;
	
	if (!(end = (char *) src)) {
		return MPT_ERROR(BadArgument);
	}
	if (!*src) {
		return 0;
	}
	/* max size unsigend integer */
	tmp = strtoumax(src, &end, base);
	if (end == src) {
		/* accept space as empty string */
		while (*src) {
			if (!isspace(*src++)) {
				return MPT_ERROR(BadType);
			}
		}
		return 0;
	}
	switch (vlen) {
	  case sizeof(int8_t) :
		if (tmp > UINT8_MAX) {
			return MPT_ERROR(BadValue);
		}
		if (val) *((int8_t *) val) = tmp;
		break;
	  case sizeof(int16_t):
		if (tmp > UINT16_MAX) {
			return MPT_ERROR(BadValue);
		}
		if (val) *((int16_t *) val) = tmp;
		break;
	  case sizeof(int32_t):
		if (tmp > UINT32_MAX) {
			return MPT_ERROR(BadValue);
		}
		if (val) *((int32_t *) val) = tmp;
		break;
	  case sizeof(int64_t):
		if (tmp > UINT64_MAX) {
			return MPT_ERROR(BadValue);
		}
		if (val) *((int64_t *) val) = tmp;
		break;
	  default:
		return MPT_ERROR(BadType);
	}
	return end - src;
}
