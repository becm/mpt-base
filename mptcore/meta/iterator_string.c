/*!
 * set metatype data.
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <sys/uio.h>

#include "meta.h"
#include "convert.h"

#include "object.h"


struct parseIterator {
	MPT_INTERFACE(iterator) ctl;
	char *val, *end, *restore, save;
};
/* iterator interface */
static int parseGet(MPT_INTERFACE(iterator) *ctl, int type, void *dest)
{
	struct parseIterator *it = (void *) ctl;
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
		return MPT_ERROR(BadValue);
	}
	/* return full remaining data */
	if (type == 's') {
		while (isspace(*txt)) ++txt;
		if (dest) *((const char **) dest) = txt;
		it->restore = 0;
		return 's';
	}
	if (type == MPT_type_vector('c')) {
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
static int parseAdvance(MPT_INTERFACE(iterator) *ctl)
{
	struct parseIterator *it = (void *) ctl;
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
static int parseReset(MPT_INTERFACE(iterator) *ctl)
{
	struct parseIterator *it = (void *) ctl;
	
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
/* reference interface */
static void parseUnref(MPT_INTERFACE(reference) *ctl)
{
	free(ctl);
}
static uintptr_t parseRef(MPT_INTERFACE(reference) *ctl)
{
	(void) ctl;
	return 0;
}
/* metatype interface */
static int parseConv(const MPT_INTERFACE(metatype) *mt, int type, void *dest)
{
	struct parseIterator *it = (void *) (mt + 1);
	
	if (!type) {
		static const uint8_t fmt[] = { MPT_ENUM(TypeIterator), 's', 0 };
		if (dest) {
			*((const uint8_t **) dest) = fmt;
			return 0;
		}
		return MPT_ENUM(TypeIterator);
	}
	if (type == 's') {
		*((const char **) dest) = (char *) (it + 1);
		return 's';
	}
	if (type == MPT_ENUM(TypeVector)
	    || type == MPT_type_vector('c')) {
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
	if (type == MPT_ENUM(TypeIterator)) {
		if (dest) *((void **) dest) = it;
		return 's';
	}
	return MPT_ERROR(BadType);
}
static MPT_INTERFACE(metatype) *parseClone(const MPT_INTERFACE(metatype) *mt)
{
	struct parseIterator *it = (void *) (mt + 1);
	const char *ptr;
	
	if (it->restore) {
		ptr = it->restore;
	} else {
		ptr = it->val;
	}
	return mpt_iterator_string(ptr, (char *) (it + 1));
}

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
extern MPT_INTERFACE(metatype) *mpt_iterator_string(const char *val, const char *sep)
{
	static const MPT_INTERFACE_VPTR(metatype) ctlMeta = {
		{ parseUnref, parseRef },
		parseConv,
		parseClone
	};
	static const MPT_INTERFACE_VPTR(iterator) ctlIter = {
		parseGet,
		parseAdvance,
		parseReset
	};
	MPT_INTERFACE(metatype) *mt;
	struct parseIterator *it;
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
	if (!(mt = malloc(sizeof(*mt) + sizeof(*it) + slen + vlen + 2))) {
		return 0;
	}
	mt->_vptr = &ctlMeta;
	it = (void *) (mt + 1);
	
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
	it->ctl._vptr = &ctlIter;
	it->val = dest;
	it->end = dest + vlen;
	it->restore = 0;
	it->save = 0;
	
	return mt;
}
