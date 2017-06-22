/*!
 * set metatype data.
 */

#include <string.h>

#include <stdarg.h>

#include "object.h"

/*!
 * \ingroup mptObject
 * \brief set object property
 * 
 * Set property to data in argument list.
 * 
 * \param obj    object interface descriptor
 * \param prop   name of property to change
 * \param format argument format
 * \param va     argument list
 */
extern int mpt_object_vset(MPT_INTERFACE(object) *obj, const char *prop, const char *format, va_list va)
{
	MPT_STRUCT(value) val;
	const char *fmt = format;
	uint8_t buf[1024];
	size_t len = 0;
	
	while (*fmt) {
		union {
			int8_t   b;
			uint8_t   y;
			int16_t n;
			uint16_t q;
			int32_t i;
			uint32_t u;
			int64_t x;
			uint64_t t;
			
			long l;
			
			float f;
			double d;
#ifdef _MPT_FLOAT_EXTENDED_H
			long double e;
#endif
			void *p;
		} val;
		int curr;
		
		if ((curr = mpt_valsize(*fmt)) < 0) {
			return MPT_ERROR(BadOperation);
		}
		switch (*fmt) {
		  case 'b': val.b = va_arg(va, int32_t); break;
		  case 'y': val.y = va_arg(va, uint32_t); break;
		  case 'n': val.n = va_arg(va, int32_t); break;
		  case 'q': val.q = va_arg(va, uint32_t); break;
		  case 'i': val.i = va_arg(va, int32_t); break;
		  case 'u': val.u = va_arg(va, uint32_t); break;
		  case 'x': val.x = va_arg(va, int64_t); break;
		  case 't': val.t = va_arg(va, uint64_t); break;
		  
		  case 'l': val.l = va_arg(va, long); break;
		  
		  case 'f': val.f = va_arg(va, double); break;
		  case 'd': val.d = va_arg(va, double); break;
#ifdef _MPT_FLOAT_EXTENDED_H
		  case 'e': val.e = va_arg(va, long double); break;
#endif
		  default:
			if (!curr) {
				val.p = va_arg(va, void *);
				curr = sizeof(val.p);
				break;
			}
			return MPT_ERROR(BadType);
		}
		if ((sizeof(buf) - len) < (size_t) curr) {
			return MPT_ERROR(BadOperation);
		}
		memcpy(buf + len, &val, curr);
		len += curr;
		++fmt;
	}
	val.fmt = format;
	val.ptr = buf;
	
	return mpt_object_iset(obj, prop, &val);
}

/*!
 * \ingroup mptMeta
 * \brief set object property
 * 
 * Set property to data in arguments according to @fmt.
 * 
 * \param obj   object interface descriptor
 * \param prop  name of property to change
 * \param fmt   argument format
 */
extern int mpt_object_set(MPT_INTERFACE(object) *obj, const char *prop, const char *fmt, ...)
{
	va_list va;
	int ret;
	
	if (!fmt) {
		return obj->_vptr->setProperty(obj, prop, 0);
	}
	va_start(va, fmt);
	ret = mpt_object_vset(obj, prop, fmt, va);
	va_end(va);
	
	return ret;
}
