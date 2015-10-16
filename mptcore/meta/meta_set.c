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


struct propParam {
	MPT_STRUCT(value) val;
	const char *sep;
	int (*conv)(const char **, int , void *);
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
static int fromText(struct propParam *par, int type, void *dest)
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
	} else if (*par->val.fmt == ((char) (MPT_ENUM(TypeVector) | 'c'))) {
		const struct iovec *vec = par->val.ptr;
		if (type == *par->val.fmt || type == MPT_ENUM(TypeVector)) {
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
	len = par->conv ? par->conv(&txt, type, dest) : stringConvert(&txt, par->sep, type, dest);
	if (len <= 0 || !dest) {
		return len;
	}
	par->val.fmt = 0;
	par->val.ptr = txt;
	
	return len;
}
#endif

static int propConv(MPT_INTERFACE(source) *ctl, int type, void *dest)
{
	struct propParam *src = (void *) (ctl + 1);
	const char *txt;
	int len;
	
	if (src->val.fmt) {
		const void *from = src->val.ptr;
#ifdef MPT_NO_CONVERT
		if (!src->val.fmt[0] || type != src->val.fmt[0]) {
			return -1;
		}
		if ((len = mpt_valsize(type)) <= 0) {
			return -3;
		}
		if (!dest) return len;
		if (!len) len = sizeof(void *);
		
		memcpy(dest, from, len);
		from = ((uint8_t *) from) + len;
#else
		if ((len = mpt_data_convert(&from, *src->val.fmt, dest, type)) < 0) {
			return fromText(src, type, dest);
		}
#endif
		if (dest) {
			src->val.ptr = from;
			++src->val.fmt;
		}
		return len;
	}
	if (!(txt = src->val.ptr)) {
		if (type != 'k' && type != 's') {
			return -3;
		}
		if (dest) ((char **) dest)[0] = 0;
		return 0;
	}
	len = src->conv ? src->conv(&txt, type, dest) : stringConvert(&txt, src->sep, type, dest);
	
	if (len <= 0 || !dest) {
		return len;
	}
	src->val.ptr = txt;
	return len;
}

static const MPT_INTERFACE_VPTR(source) _prop_vptr = {
	propConv
};


/*!
 * \ingroup mptMeta
 * \brief set solver property
 * 
 * Set solver parameter.
 * 
 * \param gen  solver descriptor
 * \param par  parameter to change
 * \param fmt  data format
 * \param data address of data to set
 */
extern int mpt_meta_pset(MPT_INTERFACE(metatype) *meta, MPT_INTERFACE(property) *pr, int (*conv)(const char **, int ,void *))
{
	struct {
		MPT_INTERFACE(source) ctl;
		struct propParam d;
	} src;
	
	src.ctl._vptr = &_prop_vptr;
	src.d.val = pr->val;
	src.d.sep = pr->desc ? pr->desc : " ,;/:";
	src.d.conv = conv;
	
	return meta->_vptr->property(meta, pr, &src.ctl);
}

/*!
 * \ingroup mptMeta
 * \brief set metatype property
 * 
 * Create contigous data format from argument list and set property.
 * 
 * \param gen  solver descriptor
 * \param par  parameter to change
 * \param fmt  argument format
 * \param va   argument list
 */
extern int mpt_meta_vset(MPT_INTERFACE(metatype) *m, const char *par, const char *fmt, va_list va)
{
	MPT_STRUCT(property) prop;
	uint8_t buf[1024];
	size_t len = 0;
	
	prop.name = par;
	prop.desc = 0;
	prop.val.fmt = fmt;
	prop.val.ptr = buf;
	
	while (*fmt) {
		int curr;
		
		if ((curr = mpt_valsize(*fmt)) < 0) {
			if (par && (curr = m->_vptr->property(m, &prop, 0)) < 0) {
				return curr;
			}
			errno = EINVAL;
			return -2;
		}
		if (!curr) {
			curr = sizeof(void*);
		}
		if ((sizeof(buf) - len) < (size_t) curr) {
			if (par && (curr = m->_vptr->property(m, &prop, 0)) < 0) {
				return curr;
			}
			errno = EOVERFLOW;
			return -2;
		}
		switch (curr) {
		  case sizeof(uint8_t):
			if (isupper(*fmt)) {
				*((uint8_t *) (buf+len)) = va_arg(va, uint32_t);
			} else {
				*((int8_t  *) (buf+len)) = va_arg(va, int32_t);
			}
			break;
		  case sizeof(uint16_t):
			if (isupper(*fmt)) {
				*((uint16_t *) (buf+len)) = va_arg(va, uint32_t);
			} else {
				*((int16_t  *) (buf+len)) = va_arg(va, int32_t);
			}
			break;
		  case sizeof(uint32_t): *((uint32_t *) (buf+len)) = va_arg(va, uint32_t); break;
		  case sizeof(uint64_t): *((uint64_t *) (buf+len)) = va_arg(va, uint64_t); break;
#ifdef _MPT_FLOAT_EXTENDED_H
		  case sizeof(long double): *((long double *) (buf+len)) = va_arg(va, long double); break;
#endif
		  default: errno = EINVAL; return -2;
		}
		len += curr;
		++fmt;
	}
	if (par) {
		return mpt_meta_pset(m, &prop, 0);
	} else {
		struct {
			MPT_INTERFACE(source) ctl;
			struct propParam d;
		} src;
		
		src.ctl._vptr = &_prop_vptr;
		src.d.val = prop.val;
		src.d.sep = " ,;/:";
		src.d.conv = 0;
		
		return m->_vptr->property(m, 0, &src.ctl);
	}
}

/*!
 * \ingroup mptMeta
 * \brief set metatype property
 * 
 * Create contigous data format from argument list and set property.
 * 
 * \param gen  solver descriptor
 * \param par  parameter to change
 * \param fmt  argument format
 * \param va   argument list
 */
extern int mpt_meta_set(MPT_INTERFACE(metatype) *m, const char *par, const char *fmt, ...)
{
	va_list va;
	int ret;
	
	va_start(va, fmt);
	if (fmt[0] == 's' && !fmt[1]) {
		if (par) {
			MPT_STRUCT(property) prop;
			
			prop.name = par ? par : "";
			prop.desc = 0;
			prop.val.fmt = 0;
			prop.val.ptr = va_arg(va, void *);
			
			ret = mpt_meta_pset(m, &prop, 0);
		} else {
			struct {
				MPT_INTERFACE(source) ctl;
				struct propParam d;
			} src;
			
			src.ctl._vptr = &_prop_vptr;
			src.d.val.fmt = 0;
			src.d.val.ptr = va_arg(va, void *);
			src.d.sep = " ,;/:";
			src.d.conv = 0;
			
			ret = m->_vptr->property(m, 0, &src.ctl);
		}
	} else {
		ret = mpt_meta_vset(m, par, fmt, va);
	}
	va_end(va);
	
	return ret;
}
