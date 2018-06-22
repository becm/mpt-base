/*!
 * set metatype data.
 */

#include <stdarg.h>
#include <inttypes.h>

#include "convert.h"

#include "meta.h"

/*!
 * \ingroup mptConvert
 * \brief set scalar value
 * 
 * Set value to next value in arguments.
 * 
 * \param len number of elements
 * \param val start address of data
 */
extern int mpt_scalar_argv(MPT_STRUCT(scalar) *s, int fmt, va_list va)
{
	size_t len;
	if (MPT_type_isVector(fmt)) {
		s->val.v = va_arg(va, struct iovec);
		s->len = sizeof(struct iovec);
		s->type = fmt;
		return fmt;
	}
	switch (fmt) {
		case 'b': s->val.b = va_arg(va, int);          len = sizeof(int8_t);   break;
		case 'y': s->val.y = va_arg(va, unsigned int); len = sizeof(uint8_t);  break;
		case 'n': s->val.n = va_arg(va, int);          len = sizeof(int16_t);  break;
		case 'q': s->val.q = va_arg(va, unsigned int); len = sizeof(uint16_t); break;
		
		case 'i': s->val.i = va_arg(va, int32_t);  len = sizeof(int32_t);  break;
		case 'u': s->val.u = va_arg(va, uint32_t); len = sizeof(uint32_t); break;
		case 'x': s->val.x = va_arg(va, int64_t);  len = sizeof(int64_t);  break;
		case 't': s->val.t = va_arg(va, uint64_t); len = sizeof(uint64_t); break;
		
		case 'l': s->val.l = va_arg(va, long); len = sizeof(long); fmt = mpt_type_int(len); break;
		
		case 'f': s->val.f = va_arg(va, double); len = sizeof(float);  break;
		case 'd': s->val.d = va_arg(va, double); len = sizeof(double); break;
#ifdef _MPT_FLOAT_EXTENDED_H
		case 'e': s->val.e = va_arg(va, long double); len = sizeof(long double); break;
#endif
		case 's':
		case 'k':
		case 'o': s->val.p = va_arg(va, void *); len = sizeof(void *); break;
		default:
			return MPT_ERROR(BadType);
	}
	s->len = len;
	s->type = fmt;
	return len;
}
