/*!
 * set metatype data.
 */

#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include <sys/uio.h>

#include "meta.h"
#include "convert.h"

#include "object.h"


struct valSource {
	MPT_INTERFACE(iterator) ctl;
	MPT_STRUCT(value) val, save;
};

static int stringConvert(const char *from, int type, void *dest)
{
	if (type == 'k') {
		const char *key, *txt = from;
		size_t klen;
		if (!(key = mpt_convert_key(&txt, 0, &klen))) {
			return MPT_ERROR(BadValue);
		}
		if (dest) {
			((const char **) dest)[0] = key;
		}
		return txt - from;
	}
#ifdef MPT_NO_CONVERT
	return MPT_ERROR(BadType);
#else
	return mpt_convert_string(from, type, dest);
#endif
}
static int fromText(const MPT_STRUCT(value) *val, int type, void *dest)
{
	const char *txt;
	int len;
	
	/* convert simple text data */
	if (*val->fmt == 's') {
		txt = *(void * const *) val->ptr;
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
	/* convert from character array */
	else if (*val->fmt == MPT_value_toVector('c')) {
		const struct iovec *vec = val->ptr;
		/* target is vector type */
		if (type == *val->fmt
		 || type == MPT_ENUM(TypeVector)) {
			if (dest) {
				*((struct iovec *) dest) = *vec;
			}
			return type;
		}
		if (!(txt = vec->iov_base)
		 || !vec->iov_len) {
			txt = 0;
		}
		/* text data not terminated */
		else if (!memchr(txt, 0, vec->iov_len)) {
			return MPT_ERROR(BadType);
		}
		if (type == 's') {
			if (dest) {
				*(void **) dest = vec->iov_base;
			}
			return type;
		}
	}
	/* not a valid string source type */
	else {
		return MPT_ERROR(BadType);
	}
	/* try to get target value from source string */
	if ((len = stringConvert(txt, type, dest)) < 0) {
		return len;
	}
	return 's';
}
static void valError(MPT_INTERFACE(unrefable) *ctl)
{
	mpt_log(0, "mpt::iterator::value", MPT_LOG(Critical), "%s (%" PRIxPTR ")",
	        MPT_tr("tried to unref temporary iterator"), ctl);
}
static int valGet(MPT_INTERFACE(iterator) *ctl, int type, void *dest)
{
	struct valSource *it = (void *) ctl;
	int len;
	uint8_t ftype;
	
	if (!type) {
		if (dest) {
			*((MPT_STRUCT(value) *) dest) = it->save;
		}
		return it->val.fmt - it->save.fmt;
	}
	/* indicate consumed contents */
	if (!it->val.fmt) {
		if (type != 's') {
			return MPT_ERROR(BadType);
		}
		if (!it->val.ptr) {
			return 0;
		}
		if (dest) {
			*((const char **) dest) = it->val.ptr;
		}
		return 's';
	}
	if (!(ftype = *it->val.fmt)) {
		return 0;
	}
	if (ftype == MPT_ENUM(TypeMeta)) {
		MPT_INTERFACE(metatype) *mt;
		if (!(mt = *((MPT_INTERFACE(metatype) **) it->val.ptr))) {
			return MPT_ERROR(BadValue);
		}
		return mt->_vptr->conv(mt, type, dest);
	}
	/* copy scalar or untracked pointer data */
	if (type == ftype
	 && MPT_value_isBasic(ftype)) {
		if ((len = mpt_valsize(ftype)) < 0) {
			return MPT_ERROR(BadArgument);
		}
		if (!len) {
			len = sizeof(void *);
		}
		if (dest) {
			memcpy(dest, it->val.ptr, len);
		}
		return ftype;
	}
	return fromText(&it->val, type, dest);
}
static int valAdvance(MPT_INTERFACE(iterator) *ctl)
{
	struct valSource *it = (void *) ctl;
	int adv;
	
	if (!it->val.fmt) {
		if (!it->val.ptr) {
			return MPT_ERROR(MissingData);
		}
		it->val.ptr = 0;
		return 0;
	}
	if (!(adv = *it->val.fmt)) {
		it->val.fmt = 0;
		it->val.ptr = 0;
		return 0;
	}
	if ((adv = mpt_valsize(adv)) < 0) {
		return MPT_ERROR(BadType);
	}
	if (!adv) {
		adv = sizeof(void *);
	}
	it->val.ptr = ((uint8_t *) it->val.ptr) + adv;
	++it->val.fmt;
	return MPT_ENUM(TypeValue);
}
static int valReset(MPT_INTERFACE(iterator) *ctl)
{
	struct valSource *it = (void *) ctl;
	it->val = it->save;
	return strlen(it->val.fmt);
}


/*!
 * \ingroup mptMeta
 * \brief dispatch value via iterator
 * 
 * Use temporary iterator instance to access
 * value data.
 * 
 * \param val  data to set
 * \param proc iterator process function
 * \param ctx  context for iterator handler
 * 
 * \return value of assignment operation
 */
extern int mpt_process_value(MPT_STRUCT(value) *val, int (*proc)(void *, MPT_INTERFACE(iterator) *), void *ctx)
{
	static const MPT_INTERFACE_VPTR(iterator) ctl = {
		{ valError },
		valGet,
		valAdvance,
		valReset
	};
	struct valSource it;
	int ret;
	
	if (!proc) {
		return MPT_ERROR(BadArgument);
	}
	it.ctl._vptr = &ctl;
	it.val = *val;
	it.save = *val;
	
	if ((ret = proc(ctx, &it.ctl)) >= 0) {
		*val = it.val;
	}
	return ret;
}

static void valUnref(MPT_INTERFACE(unrefable) *ctl)
{
	free(ctl);
}
static const MPT_INTERFACE_VPTR(iterator) _val_vptr = {
	{ valUnref },
	valGet,
	valAdvance,
	valReset
};

/*!
 * \ingroup mptObject
 * \brief create iterator from value
 * 
 * New iterator with shallow copy of value metadata
 * or deep copy of value format and content.
 * 
 * \param val  data to set
 * \param pos  current iterator position
 * 
 * \return new iterator representating value
 */
extern MPT_INTERFACE(iterator) *mpt_iterator_value(MPT_STRUCT(value) val, int pos)
{
	struct valSource *it;
	const char *fmt;
	char *data;
	size_t vlen, flen, vpos;
	int type, curr;
	
	if (!(fmt = val.fmt) || !val.ptr) {
		errno = EINVAL;
		return 0;
	}
	/* shallow copy of value metadata */
	if (pos < 0) {
		if (!(it = malloc(sizeof(*it)))) {
			return 0;
		}
		it->ctl._vptr = &_val_vptr;
		it->val = val;
		it->save = val;
		return &it->ctl;
	}
	vlen = 0;
	flen = 0;
	vpos = 0;
	curr = 0;
	while ((type = fmt[flen++])) {
		if (!MPT_value_isBasic(type)) {
			errno = EINVAL;
			return 0;
		}
		if ((type = mpt_valsize(*fmt)) < 0) {
			errno = EINVAL;
			return 0;
		}
		if (curr < pos) {
			++curr;
			vpos += type;
		}
		vlen += type;
	}
	/* create iterator with local format and data */
	if (!(it = malloc(sizeof(*it) + flen + vlen))) {
		return 0;
	}
	it->ctl._vptr = &_val_vptr;
	
	/* save base and current format info */
	data = memcpy(it + 1, val.fmt, flen);
	it->save.fmt = data;
	it->val.fmt = data + curr;
	
	/* save base and current data content */
	data = memcpy(data + flen, val.ptr, vlen);
	it->save.ptr = data;
	it->val.ptr = data + vpos;
	
	return &it->ctl;
}
