/*!
 * set metatype data.
 */

#include <string.h>

#include <stdarg.h>

#include "convert.h"
#include "core.h"

/*!
 * \ingroup mptMeta
 * \brief set object property
 * 
 * Set property to data in argument list.
 * 
 * \param obj   object interface descriptor
 * \param prop  name of property to change
 * \param fmt   argument format
 * \param va    argument list
 */
extern int mpt_object_vset(MPT_INTERFACE(object) *obj, const char *prop, const char *format, va_list va)
{
	MPT_STRUCT(value) val;
	const char *fmt = format;
	uint8_t buf[1024];
	size_t len = 0;
	
	while (*fmt) {
		int curr;
		
		if ((curr = mpt_valsize(*fmt)) < 0) {
			return MPT_ERROR(BadOperation);
		}
		if ((sizeof(buf) - len) < (curr ? (size_t) curr : sizeof(void*))) {
			return MPT_ERROR(BadOperation);
		}
		switch (*fmt) {
		  case 'b': *((int8_t  *)   (buf+len)) = va_arg(va, int32_t); break;
		  case 'y': *((uint8_t  *)  (buf+len)) = va_arg(va, uint32_t); break;
		  case 'n': *((int16_t *)   (buf+len)) = va_arg(va, int32_t); break;
		  case 'q': *((uint16_t *)  (buf+len)) = va_arg(va, uint32_t); break;
		  case 'i': *((int32_t *)   (buf+len)) = va_arg(va, int32_t); break;
		  case 'u': *((uint32_t *)  (buf+len)) = va_arg(va, uint32_t); break;
		  case 'x': *((int64_t *)   (buf+len)) = va_arg(va, int64_t); break;
		  case 't': *((uint64_t *)  (buf+len)) = va_arg(va, uint64_t); break;
		  
		  case 'l': *((long *)  (buf+len)) = va_arg(va, long); break;
		  
		  case 'f': *((float *) (buf+len))  = va_arg(va, double); break;
		  case 'd': *((double *) (buf+len)) = va_arg(va, double); break;
#ifdef _MPT_FLOAT_EXTENDED_H
		  case 'e': *((long double *) (buf+len)) = va_arg(va, long double); break;
#endif
		  default:
			if (!curr) {
				void *ptr = va_arg(va, void *);
				memcpy(buf+len, &ptr, curr = sizeof(ptr));
				break;
			}
			return MPT_ERROR(BadType);
		}
		len += curr;
		++fmt;
	}
	val.fmt = format;
	val.ptr = buf;
	
	return mpt_object_pset(obj, prop, &val, 0);
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
		mpt_object_pset(obj, prop, 0, 0);
	}
	va_start(va, fmt);
	if (fmt[0] == 's' && !fmt[1]) {
		MPT_STRUCT(value) val;
		
		val.fmt = 0;
		val.ptr = va_arg(va, void *);
		
		ret = mpt_object_pset(obj, prop, &val, 0);
	} else {
		ret = mpt_object_vset(obj, prop, fmt, va);
	}
	va_end(va);
	
	return ret;
}
