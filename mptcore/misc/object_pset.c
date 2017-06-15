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
	MPT_STRUCT(value) val, save;
	const char *sep;
	int advance;
};

static int stringConvert(const char *from, const char *sep, int type, void *dest)
{
	if (type == 'k') {
		const char *key, *txt = from;
		size_t klen;
		if (!(key = mpt_convert_key(&txt, sep, &klen))) {
			return MPT_ERROR(BadValue);
		}
		if (dest) {
			((const char **) dest)[0] = key;
		}
		return txt - from;
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
			if (dest) {
				*(const void **) dest = txt;
			}
			return type;
		}
		if (!txt) {
			return MPT_ERROR(BadValue);
		}
	}
	else if (*par->val.fmt == MPT_value_toVector('c')) {
		const struct iovec *vec = par->val.ptr;
		if (type == *par->val.fmt
		    || type == MPT_ENUM(TypeVector)) {
			if (dest) {
				*((struct iovec *) dest) = *vec;
			}
			return type;
		}
		/* text data not terminated */
		if (!(txt = vec->iov_base)
		    || !memchr(txt, 0, vec->iov_len)) {
			return MPT_ERROR(BadType);
		}
		if (type == 's') {
			if (dest) {
				*(void **) dest = vec->iov_base;
			}
			return type;
		}
	} else {
		return MPT_ERROR(BadType);
	}
	if ((len = stringConvert(txt, par->sep, type, dest)) < 0) {
		return len;
	}
	return 's';
}
#endif

static void propUnref(MPT_INTERFACE(unrefable) *ctl)
{
	(void) ctl;
}
static int propConv(const MPT_INTERFACE(metatype) *ctl, int type, void *dest)
{
	struct paramSource *src = (void *) ctl;
	const char *txt;
	int len;
	
	/* indicate consumed value */
	if (!src->val.ptr) {
		return 0;
	}
	if (type == MPT_ENUM(TypeIterator)) {
		if (dest) *((void **) dest) = src;
		return MPT_ENUM(TypeValue);
	}
	if (type == MPT_ENUM(TypeValue)) {
		if (dest) memcpy(dest, &src->val, sizeof(src->val));
		return type;
	}
	if (src->val.fmt) {
		const void *from = src->val.ptr;
		uint8_t ftype = *src->val.fmt;
		
		if (!ftype) {
			return 0;
		}
#ifdef MPT_NO_CONVERT
		if (type != ftype) {
			return MPT_ERROR(BadValue);
		}
		if ((len = mpt_valsize(dtype)) < 0) {
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
		return len;
	}
	if (!(txt = src->val.ptr)) {
		if (type == 'k' || type == 's') {
			if (dest) ((char **) dest)[0] = 0;
			return 's';
		}
		return 0;
	}
	len = stringConvert(txt, src->sep, type, dest);
	
	if (len < 0) {
		return len;
	}
	src->advance = len;
	
	return len ? 's' : 0;
}
static MPT_INTERFACE(metatype) *propClone(const MPT_INTERFACE(metatype) *ctl)
{
	(void) ctl; return 0;
}
static int propAdvance(MPT_INTERFACE(iterator) *ctl)
{
	struct paramSource *src = (void *) ctl;
	const char *base;
	int adv;
	
	if (src->val.fmt) {
		if (!(adv = *src->val.fmt)) {
			return MPT_ERROR(MissingData);
		}
		if ((adv = mpt_valsize(adv)) < 0) {
			return MPT_ERROR(BadType);
		}
		src->val.ptr = ((uint8_t *) src->val.ptr) + adv;
		++src->val.fmt;
		return MPT_ENUM(TypeValue);
	}
	if (!(base = src->val.ptr)
	    || !src->sep) {
		return MPT_ERROR(MissingData);
	}
#ifdef MPT_NO_CONVERT
	return MPT_ERROR(BadType);
#else
	if (!mpt_convert_key(&base, src->sep, 0)) {
		return MPT_ERROR(MissingData);
	}
	src->val.ptr = base;
	return 's';
#endif
}
static int propReset(MPT_INTERFACE(iterator) *ctl)
{
	struct paramSource *src = (void *) ctl;
	src->val = src->save;
	src->advance = 0;
	return 0;
}
static const MPT_INTERFACE_VPTR(iterator) _prop_vptr = {
	{ { propUnref }, propConv, propClone },
	propAdvance,
	propReset
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
	
	src.ctl._vptr = &_prop_vptr.meta;
	
	if (!val) {
		src.sep = 0;
	} else {
		src.val = *val;
		src.save = *val;
		src.sep = sep ? sep : " ,;/:";
	}
	return obj->_vptr->setProperty(obj, name, &src.ctl);
}
