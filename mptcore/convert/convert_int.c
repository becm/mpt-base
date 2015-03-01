/*!
 * get signed integer value from string
 */

#include <stdlib.h>
#include <inttypes.h>
#include <limits.h>
#include <errno.h>

#include "convert.h"

#define GET_STRING_FCN(get, name, type, vmin, vmax) \
int name(type *val, const char *src, int base, const type range[2]) \
{ \
	type trange[2] = { vmin, vmax }, tmp; \
	int ret; \
	\
	if ((ret = get(&tmp, sizeof(type), src, base)) <= 0) \
		return ret; \
	\
	if (!range) range = trange; \
	\
	if (tmp < range[0] || tmp > range[1]) { \
		errno = ERANGE; return -2; \
	} \
	if (val) *val = tmp; \
	return ret; \
}

GET_STRING_FCN(_mpt_convert_int, mpt_cint8,  int8_t,  INT8_MIN,  INT8_MAX )
GET_STRING_FCN(_mpt_convert_int, mpt_cint16, int16_t, INT16_MIN, INT16_MAX)
GET_STRING_FCN(_mpt_convert_int, mpt_cint32, int32_t, INT32_MIN, INT32_MAX)
GET_STRING_FCN(_mpt_convert_int, mpt_cint64, int64_t, INT64_MIN, INT64_MAX)

GET_STRING_FCN(_mpt_convert_int, mpt_cchar,  char,    CHAR_MIN,  CHAR_MAX)
GET_STRING_FCN(_mpt_convert_int, mpt_cint,   int,     INT_MIN,   INT_MAX )
GET_STRING_FCN(_mpt_convert_int, mpt_clong,  long,    LONG_MIN,  LONG_MAX)

extern int _mpt_convert_int(void *val, size_t vlen, const char *src, int base)
{
	intmax_t tmp;
	char *end;
	
	if (!(end = (char *) src)) {
		errno = EFAULT; return -1;
	}
	/* max size unsigend integer */
	tmp = *src ? strtoimax(src, &end, base) : 0;
	if (*src && end == src) return -1;
	if (!val) return end - src;
	
	switch (vlen) {
	  case sizeof(int8_t) : *((uint8_t  *) val) = tmp; break;
	  case sizeof(int16_t): *((uint16_t *) val) = tmp; break;
	  case sizeof(int32_t): *((uint32_t *) val) = tmp; break;
	  case sizeof(int64_t): *((uint64_t *) val) = tmp; break;
	  default: errno = EINVAL; return -3;
	}
	
	return end - src;
}

GET_STRING_FCN(_mpt_convert_uint, mpt_cuint8,  uint8_t,  0, UINT8_MAX )
GET_STRING_FCN(_mpt_convert_uint, mpt_cuint16, uint16_t, 0, UINT16_MAX)
GET_STRING_FCN(_mpt_convert_uint, mpt_cuint32, uint32_t, 0, UINT32_MAX)
GET_STRING_FCN(_mpt_convert_uint, mpt_cuint64, uint64_t, 0, UINT64_MAX)

GET_STRING_FCN(_mpt_convert_uint, mpt_cuchar,  unsigned char, 0, UCHAR_MAX)
GET_STRING_FCN(_mpt_convert_uint, mpt_cuint,   unsigned int,  0, UINT_MAX )
GET_STRING_FCN(_mpt_convert_uint, mpt_culong,  unsigned long, 0, ULONG_MAX)

extern int _mpt_convert_uint(void *val, size_t vlen, const char *src, int base)
{
	uintmax_t tmp;
	char	*end;
	
	if (!(end = (char *) src)) {
		errno = EFAULT; return -1;
	}
	/* max size unsigend integer */
	tmp = *src ? strtoumax(src, &end, base) : 0;
	if (*src && end == src) return -1;
	if (!val) return end - src;
	
	switch (vlen) {
	  case sizeof(uint8_t) : *((uint8_t  *) val) = tmp; break;
	  case sizeof(uint16_t): *((uint16_t *) val) = tmp; break;
	  case sizeof(uint32_t): *((uint32_t *) val) = tmp; break;
	  case sizeof(uint64_t): *((uint64_t *) val) = tmp; break;
	  default: errno = EINVAL; return -3;
	}
	
	return end - src;
}
