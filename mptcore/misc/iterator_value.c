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
	MPT_INTERFACE(iterator) _it;
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

/* iterator operations */
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
static const MPT_INTERFACE_VPTR(iterator) _vptr_iterator_value = {
	valGet,
	valAdvance,
	valReset
};


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
	struct valSource it;
	int ret;
	
	if (!proc) {
		return MPT_ERROR(BadArgument);
	}
	it._it._vptr = &_vptr_iterator_value;
	it.val = *val;
	it.save = *val;
	
	if ((ret = proc(ctx, &it._it)) >= 0) {
		*val = it.val;
	}
	return ret;
}

/* reference interface */
static void valUnref(MPT_INTERFACE(reference) *ctl)
{
	free(ctl);
}
static uintptr_t valRef(MPT_INTERFACE(reference) *ctl)
{
	(void) ctl;
	return 0;
}
/* metatype interface */
static int valConv(const MPT_INTERFACE(metatype) *mt, int type, void *dest)
{
	struct valSource *it = (void *) (mt + 1);
	if (!type) {
		static const char fmt[] = { MPT_ENUM(TypeIterator), MPT_ENUM(TypeValue) };
		if (dest) {
			*((const char **) dest) = fmt;
		}
		return MPT_ENUM(TypeIterator);
	}
	if (type == MPT_ENUM(TypeIterator)) {
		if (dest) {
			*((void **) dest) = &it->_it;
		}
		return MPT_ENUM(TypeIterator);
	}
	if (type == MPT_ENUM(TypeValue)) {
		if (dest) {
			*((MPT_STRUCT(value) *) dest) = it->save;
		}
		return MPT_ENUM(TypeIterator);
	}
	return MPT_ERROR(BadType);
}
static MPT_INTERFACE(metatype) *valClone(const MPT_INTERFACE(metatype) *mt)
{
	struct valSource *it = (void *) (mt + 1);
	return mpt_iterator_value(it->save, it->val.fmt - it->save.fmt);
}
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
extern MPT_INTERFACE(metatype) *mpt_iterator_value(MPT_STRUCT(value) val, int pos)
{
	static const MPT_INTERFACE_VPTR(metatype) valMeta = {
		{ valUnref, valRef },
		valConv,
		valClone
	};
	MPT_INTERFACE(metatype) *mt;
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
		if (!(mt = malloc(sizeof(*mt) + sizeof(*it)))) {
			return 0;
		}
		mt->_vptr = &valMeta;
		it = (void *) (mt + 1);
		it->_it._vptr = &_vptr_iterator_value;
		it->val = val;
		it->save = val;
		return mt;
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
	if (!(mt = malloc(sizeof(*mt) + sizeof(*it) + flen + vlen))) {
		return 0;
	}
	mt->_vptr = &valMeta;
	it = (void *) (mt + 1);
	it->_it._vptr = &_vptr_iterator_value;
	
	/* save base and current format info */
	data = memcpy(it + 1, val.fmt, flen);
	it->save.fmt = data;
	it->val.fmt = data + curr;
	
	/* save base and current data content */
	data = memcpy(data + flen, val.ptr, vlen);
	it->save.ptr = data;
	it->val.ptr = data + vpos;
	
	return mt;
}
