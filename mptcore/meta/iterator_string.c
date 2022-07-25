/*!
 * set metatype data.
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <sys/uio.h>

#include "meta.h"
#include "convert.h"
#include "types.h"

#include "object.h"


MPT_STRUCT(parseIterator) {
	MPT_INTERFACE(metatype) _mt;
	MPT_INTERFACE(iterator) _it;
	
	struct {
		MPT_INTERFACE(convertable) _conv;
		MPT_STRUCT(value) val;
	} elem;
	
	MPT_INTERFACE(convertable) *conv_ptr;
	
	char *val, *end, *restore, save;
};
/* iterator interface */
static int parseConvertElement(MPT_INTERFACE(convertable) *conv, int type, void *dest)
{
	MPT_STRUCT(parseIterator) *it = MPT_baseaddr(parseIterator, conv, elem._conv);
	const char *txt;
	int len;
	
	/* indicate consumed value */
	if (!(txt = it->val)) {
		return 0;
	}
	/* reset value end indicator */
	if (it->restore) {
		*it->restore = it->save;
	}
	if (!*txt) {
		if (type == 'k' || type == 's') {
			if (dest) ((char **) dest)[0] = 0;
			it->restore = 0;
			return 's';
		}
		if (it->restore) {
			it->restore = 0;
		}
		return MPT_ERROR(MissingData);
	}
	/* return full remaining data */
	if (type == 's') {
		while (isspace(*txt)) ++txt;
		if (dest) *((const char **) dest) = txt;
		it->restore = 0;
		return 's';
	}
	if (type == MPT_type_toVector('c')) {
		while (isspace(*txt)) ++txt;
		while (!isspace(*txt)) ++txt;
		it->restore = (char *) txt;
		it->save = *txt;
		*it->restore = 0;
		if (dest) {
			struct iovec *vec = dest;
			vec->iov_base = it->val;
			vec->iov_len = txt - it->val;
		}
		return 's';
	}
	/* consume next complete word */
	if (type == 'k') {
		const char *sep = (char *) (it + 1);
		const char *key;
		size_t klen;
		if (!(key = mpt_convert_key(&txt, *sep ? sep : 0, &klen))) {
			if (it->restore) {
				it->restore = 0;
			}
			return MPT_ERROR(BadValue);
		}
		if (dest) {
			((const char **) dest)[0] = key;
		}
		len = txt - it->val;
	}
	/* convert to target type */
	else if ((len = mpt_convert_string(it->val, type, dest)) < 0) {
		return len;
	}
	/* terminate consumed substring */
	it->restore = it->val + len;
	if (it->restore >= it->end) {
		it->restore = 0;
	} else {
		it->save = *it->restore;
		*it->restore = 0;
	}
	return 's';
}
static const MPT_STRUCT(value) *parseValue(MPT_INTERFACE(iterator) *ptr)
{
	MPT_STRUCT(parseIterator) *d = MPT_baseaddr(parseIterator, ptr, _it);
	return &d->elem.val;
}
static int parseAdvance(MPT_INTERFACE(iterator) *ptr)
{
	MPT_STRUCT(parseIterator) *it = MPT_baseaddr(parseIterator, ptr, _it);
	char *next;
	size_t len;
	
	if (!it->end) {
		return MPT_ERROR(MissingData);
	}
	if (!it->val) {
		it->end = 0;
		return 0;
	}
	len = it->end - it->val;
	if (!(len = it->end - it->val)) {
		it->val = 0;
		it->end = 0;
		return 0;
	}
	if (it->restore) {
		it->val = it->restore + 1;
		*it->restore = it->save;
		it->restore = 0;
	}
	else if ((next = memchr(it->val, 0, len))) {
		it->val = next + 1;
	}
	else {
		it->val = 0;
		return 0;
	}
	return 's';
}
static int parseReset(MPT_INTERFACE(iterator) *ptr)
{
	MPT_STRUCT(parseIterator) *it = MPT_baseaddr(parseIterator, ptr, _it);
	
	if (it->restore) {
		*it->restore = it->save;
		it->restore = 0;
	}
	/* value after separator config */
	if ((it->val = strchr((void *) (it + 1), 0))) {
		++it->val;
	}
	return 1;
}
/* convertable interface */
static int parseConv(MPT_INTERFACE(convertable) *val, int type, void *dest)
{
	MPT_STRUCT(parseIterator) *it = MPT_baseaddr(parseIterator, val, _mt);
	
	if (!type) {
		static const uint8_t fmt[] = { MPT_ENUM(TypeIteratorPtr), 's', 0 };
		if (dest) {
			*((const uint8_t **) dest) = fmt;
			return 0;
		}
		return MPT_ENUM(TypeIteratorPtr);
	}
	if (type == 's') {
		*((const char **) dest) = (char *) (it + 1);
		return 's';
	}
	if (type == MPT_ENUM(TypeVector)
	    || type == MPT_type_toVector('c')) {
		struct iovec *vec;
		if ((vec = dest)) {
			vec->iov_base = it + 1;
			if (it->restore) {
				vec->iov_len = it->restore - it->val;
			} else {
				vec->iov_len = it->end - it->val;
			}
		}
		return 's';
	}
	if (type == MPT_ENUM(TypeIteratorPtr)) {
		if (dest) *((void **) dest) = &it->_it;
		return 's';
	}
	return MPT_ERROR(BadType);
}
/* metatype interface */
static void parseUnref(MPT_INTERFACE(metatype) *mt)
{
	free(mt);
}
static uintptr_t parseRef(MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
static MPT_INTERFACE(metatype) *parseClone(const MPT_INTERFACE(metatype) *mt)
{
	MPT_STRUCT(parseIterator) *it = MPT_baseaddr(parseIterator, mt, _mt);
	const char *ptr;
	
	if (it->restore) {
		ptr = it->restore;
	} else {
		ptr = it->val;
	}
	return mpt_iterator_string(ptr, (char *) (it + 1));
}

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
extern MPT_INTERFACE(metatype) *mpt_iterator_string(const char *val, const char *sep)
{
	static const MPT_INTERFACE_VPTR(metatype) ctlMeta = {
		{ parseConv },
		parseUnref,
		parseRef,
		parseClone
	};
	static const MPT_INTERFACE_VPTR(iterator) ctlIter = {
		parseValue,
		parseAdvance,
		parseReset
	};
	static const MPT_INTERFACE_VPTR(convertable) ctlConv = {
		parseConvertElement
	};
	MPT_STRUCT(parseIterator) *it;
	size_t slen, vlen;
	char *dest;
	
	/* no need for separator */
	if (!val) {
		slen = vlen = 0;
	}
	else {
		if (!sep) {
			sep = " ,;/:";
		}
		slen = strlen(sep);
		vlen = strlen(val);
	}
	/* memory for header, constent and string separation/end */
	if (!(it = malloc(sizeof(*it) + slen + vlen + 2))) {
		return 0;
	}
	it->_mt._vptr = &ctlMeta;
	it->_it._vptr = &ctlIter;
	
	it->elem._conv._vptr = &ctlConv;
	MPT_value_set(&it->elem.val, MPT_ENUM(TypeConvertablePtr), &it->conv_ptr);
	it->conv_ptr = &it->elem._conv;
	
	/* save separator config */
	dest = (char *) (it + 1);
	if (slen) {
		memcpy(dest, sep, slen);
	}
	dest[slen++] = '\0';
	
	/* save value content */
	dest += slen;
	if (vlen) {
		memcpy(dest, val, vlen);
	}
	dest[vlen] = '\0';
	
	/* setup iterator data */
	it->val = dest;
	it->end = dest + vlen;
	it->restore = 0;
	it->save = 0;
	
	return &it->_mt;
}
