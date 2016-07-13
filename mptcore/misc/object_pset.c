/*!
 * set metatype data.
 */

#include <string.h>

#include <sys/uio.h>

#include "meta.h"
#include "convert.h"

#include "object.h"


struct paramSource {
	MPT_INTERFACE(metatype) ctl;
	const char *sep;
	MPT_STRUCT(value) val;
};

static int stringConvert(const char *from, const char *sep, int type, void *dest)
{
	if ((type & 0xff) == 'k') {
		const char *key, *txt = from;
		size_t klen;
		if (!(key = mpt_convert_key(&txt, sep, &klen))) {
			return MPT_ERROR(BadValue);
		}
		if (dest) {
			((const char **) dest)[0] = key;
		}
		if (type & MPT_ENUM(ValueConsume)) {
			return txt - from;
		}
		return key - from;
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
		if ((type & 0xff) == 's') {
			if (dest) {
				*(const void **) dest = txt;
			}
			if (type & MPT_ENUM(ValueConsume)) {
				++par->val.fmt;
				par->val.ptr = ((void **) par->val.ptr) + 1;
				return 's' | MPT_ENUM(ValueConsume);
			}
			return 's';
		}
		if (!txt) {
			return MPT_ERROR(BadValue);
		}
	}
	else if (*par->val.fmt == MPT_value_toVector('c')) {
		const struct iovec *vec = par->val.ptr;
		if ((type & 0xff) == *par->val.fmt
		    || (type & 0xff) == MPT_ENUM(TypeVecBase)) {
			if (dest) {
				*((struct iovec *) dest) = *vec;
			}
			if (type & MPT_ENUM(ValueConsume)) {
				type = *par->val.fmt++;
				par->val.ptr = vec + 1;
				return type | MPT_ENUM(ValueConsume);
			}
			return *par->val.fmt;
		}
		/* text data not terminated */
		if (!(txt = vec->iov_base)
		    || !memchr(txt, 0, vec->iov_len)) {
			return MPT_ERROR(BadType);
		}
		if ((type & 0xff) == 's') {
			if (dest) {
				*(void **) dest = vec->iov_base;
			}
			if (type & MPT_ENUM(ValueConsume)) {
				type = *par->val.fmt++;
				par->val.ptr = vec + 1;
				return type | MPT_ENUM(ValueConsume);
			}
			return *par->val.fmt;
		}
	} else {
		return MPT_ERROR(BadType);
	}
	if ((len = stringConvert(txt, par->sep, type, dest)) < 0) {
		return len;
	}
	if (type & MPT_ENUM(ValueConsume)) {
		type = *par->val.fmt++;
		par->val.ptr = ((void **) par->val.ptr) + 1;
		return type | MPT_ENUM(ValueConsume);
	}
	return *par->val.fmt;
}
#endif

static void propUnref(MPT_INTERFACE(unrefable) *ctl)
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
	len = stringConvert(txt, src->sep, type | MPT_ENUM(ValueConsume), dest);
	
	if (len < 0) {
		return len;
	}
	if (!len) {
		src->sep = 0;
		return 0;
	}
	if (type & MPT_ENUM(ValueConsume)) {
		src->val.ptr = txt + len;
		return MPT_ENUM(ValueConsume) | 's';
	}
	return 's';
}
static MPT_INTERFACE(metatype) *propClone(const MPT_INTERFACE(metatype) *ctl)
{
	(void) ctl; return 0;
}
static const MPT_INTERFACE_VPTR(metatype) _prop_vptr = {
	{ propUnref },
	propAssign,
	propConv,
	propClone
};


/*!
 * \ingroup mptObject
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
	
	if (!val) {
		src.sep = 0;
	} else {
		src.sep = sep ? sep : " ,;/:";
		src.val = *val;
	}
	return obj->_vptr->setProperty(obj, name, &src.ctl);
}
