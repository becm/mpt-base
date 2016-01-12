/*!
 * set metatype data.
 */

#include <string.h>

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
	} else if (*par->val.fmt == MPT_value_toVector('c')) {
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
	
	/* indicate consumed value */
	if (!src->sep) {
		return 0;
	}
	if ((type & 0xff) == MPT_ENUM(TypeValue)) {
		if (type & MPT_ENUM(ValueConsume)) {
			src->sep = 0;
			return MPT_ENUM(TypeValue) | MPT_ENUM(ValueConsume);
		}
		return MPT_ENUM(TypeValue);
	}
	if (src->val.fmt) {
		const void *from = src->val.ptr;
		
		if (!src->val.fmt[0]) {
			src->sep = 0;
			return 0;
		}
#ifdef MPT_NO_CONVERT
		if ((type & 0xff) != src->val.fmt[0]) {
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
			return (type & 0xff) | MPT_ENUM(ValueConsume);
		}
		return type & 0xff;
	}
	if (!(txt = src->val.ptr)) {
		if ((type & 0xff) != 'k' && (type & 0xff) != 's') {
			return -3;
		}
		if (dest) ((char **) dest)[0] = 0;
		if (type & MPT_ENUM(ValueConsume)) {
			src->sep = 0;
			return 's' | MPT_ENUM(ValueConsume);
		}
		return 's';
	}
	len = stringConvert(&txt, src->sep, type, dest);
	
	if (len < 0) {
		return len;
	}
	if (!len) {
		src->sep = 0;
		return 0;
	}
	if (type & MPT_ENUM(ValueConsume)) {
		src->val.ptr = txt + len;
		return 's' | MPT_ENUM(ValueConsume);
	}
	return 's';
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
 * \param par  name of property to change
 * \param val  data to set
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
