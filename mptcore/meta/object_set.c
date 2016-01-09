/*!
 * set metatype data.
 */

#include <ctype.h>
#include <string.h>

#include <stdarg.h>
#include <errno.h>

#include <sys/uio.h>

#include "convert.h"
#include "core.h"


struct paramSource {
	MPT_INTERFACE(metatype) ctl;
	const char *sep;
	MPT_STRUCT(value) val;
};

static int stringConvert(const char **from, const char *sep, int type, void *dest)
{
	if (type == 'k') {
		const char *txt;
		size_t klen;
		if (!(txt = mpt_convert_key(from, sep, &klen))) return -2;
		if (dest) ((const char **) dest)[0] = txt;
		return klen;
	}
#ifdef MPT_NO_CONVERT
	return -2;
#else
	return mpt_convert_string(from, type, dest);
#endif
}
#ifndef MPT_NO_CONVERT
static int fromText(struct paramSource *par, int type, void *dest)
{
	const char *txt;
	int len;
	
	if (*par->val.fmt == 's') {
		txt = *(void **) par->val.ptr;
		if (type == 's') {
			*(const void **) dest = txt;
			return txt ? strlen(txt) : 0;
		}
		if (!txt) return -2;
	} else if (*par->val.fmt == ('c' - 0x40)) {
		const struct iovec *vec = par->val.ptr;
		if (type == *par->val.fmt || type == MPT_ENUM(TypeVecBase)) {
			*((struct iovec *) dest) = *vec;
			return sizeof(*vec);
		}
		if (type == 's') {
			*(void **) dest = vec->iov_base;
			return vec->iov_len;
		}
		if (!(txt = vec->iov_base) || !memchr(txt, 0, vec->iov_len)) return -2;
	} else {
		return -3;
	}
	len = stringConvert(&txt, par->sep, type, dest);
	if (len <= 0 || !dest) {
		return len;
	}
	par->val.fmt = 0;
	par->val.ptr = txt;
	
	return len;
}
#endif

static void propUnref(MPT_INTERFACE(metatype) *ctl)
{
	(void) ctl;
}
static int propAssign(MPT_INTERFACE(metatype) *ctl, const MPT_STRUCT(value) *val)
{
	(void) ctl; (void) val; return MPT_ERROR(BadOperation);
}
static int propConv(MPT_INTERFACE(metatype) *ctl, int type, void *dest)
{
	struct paramSource *src = (void *) ctl;
	const char *txt;
	int len;
	
	if (src->val.fmt) {
		const void *from = src->val.ptr;
#ifdef MPT_NO_CONVERT
		if (!src->val.fmt[0] || (type & 0xff) != src->val.fmt[0]) {
			return MPT_ERROR(BadValue);
		}
		if ((len = mpt_valsize(type & 0xff)) < 0) {
			return MPT_ERROR(BadType);
		}
		if (!len) len = sizeof(void *);
		
		if (dest) memcpy(dest, from, len);
		from = ((uint8_t *) from) + len;
#else
		if ((len = mpt_data_convert(&from, *src->val.fmt, dest, type)) < 0) {
			return fromText(src, type, dest);
		}
#endif
		if (type & MPT_ENUM(ValueConsume)) {
			src->val.ptr = from;
			++src->val.fmt;
		}
		return len;
	}
	if (!(txt = src->val.ptr)) {
		if ((type & 0xff) != 'k' && (type & 0xff) != 's') {
			return -3;
		}
		if (dest) ((char **) dest)[0] = 0;
		return 0;
	}
	len = stringConvert(&txt, src->sep, type, dest);
	
	if (len <= 0 || !dest) {
		return len;
	}
	src->val.ptr = txt;
	return len;
}
static MPT_INTERFACE(metatype) *propClone(MPT_INTERFACE(metatype) *ctl)
{
	(void) ctl; return 0;
}
static const MPT_INTERFACE_VPTR(metatype) _prop_vptr = {
	propUnref,
	propAssign,
	propConv,
	propClone
};


/*!
 * \ingroup mptMeta
 * \brief set object property
 * 
 * Set property data to value.
 * 
 * \param obj  object interface descriptor
 * \param par  property to change
 * \param val  data format
 * \param sep  allowed keyword separators
 */
extern int mpt_object_pset(MPT_INTERFACE(object) *obj, const char *name, const MPT_STRUCT(value) *val, const char *sep)
{
	struct paramSource src;
	
	src.ctl._vptr = &_prop_vptr;
	src.sep = sep ? sep : " ,;/:";
	
	if (!val) {
		src.val.fmt = 0;
		src.val.ptr = 0;
	} else {
		src.val = *val;
	}
	return obj->_vptr->setProperty(obj, name, &src.ctl);
}

/*!
 * \ingroup mptMeta
 * \brief set object property
 * 
 * Set property to data in argument list.
 * 
 * \param obj   object interface descriptor
 * \param prop  property to change
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
			errno = EINVAL;
			return MPT_ERROR(BadOperation);
		}
		if ((sizeof(buf) - len) < (curr ? (size_t) curr : sizeof(void*))) {
			errno = EOVERFLOW;
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
			errno = EINVAL;
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
 * Set property to data in argument list.
 * 
 * \param obj   object interface descriptor
 * \param prop  property to change
 * \param fmt   argument format
 * \param va    argument list
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
