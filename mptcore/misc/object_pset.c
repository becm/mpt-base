/*!
 * set metatype data.
 */

#include <string.h>

#include <sys/uio.h>

#include "meta.h"

#include "object.h"

struct wrapIter
{
	MPT_INTERFACE(metatype) _ctl;
	MPT_INTERFACE(iterator) *it;
	const char *val;
};

static void metaIterUnref(MPT_INTERFACE(unrefable) *ref)
{
	(void) ref;
}
static MPT_INTERFACE(metatype) *metaIterClone(const MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
static int metaIterConv(const MPT_INTERFACE(metatype) *mt, int type, void *dest)
{
	struct wrapIter *it = (void *) mt;
	MPT_INTERFACE(iterator) *src = it->it;
	if (!type) {
		static const char fmt[] = { MPT_ENUM(TypeIterator), 's', 0 };
		if (dest) {
			*((const char **) dest) = fmt;
		}
		return *fmt;
	}
	if (src && type == MPT_ENUM(TypeIterator)) {
		if (dest) {
			*((const void **) dest) = src;
		}
		return 's';
	}
	if (type == MPT_value_toVector('c')) {
		struct iovec *vec;
		if ((vec = dest)) {
			vec->iov_base = (void *) it->val;
			vec->iov_len = strlen(it->val);
		}
		return 's';
	}
	if (type == 's') {
		if (dest) {
			*((const char **) dest) = it->val;
		}
		return 's';
	}
	if (src) {
		return src->_vptr->get(src, type, dest);
	}
	return MPT_ERROR(BadType);
}

/*!
 * \ingroup mptObject
 * \brief set object property
 * 
 * Set property data to value.
 * 
 * \param obj  object interface descriptor
 * \param par  name of property to change
 * \param val  string data to set
 * \param sep  allowed keyword separators
 */
extern int mpt_object_pset(MPT_INTERFACE(object) *obj, const char *name, const char *val, const char *sep)
{
	static const MPT_INTERFACE_VPTR(metatype) ctl = {
		{ metaIterUnref },
		metaIterConv,
		metaIterClone
	};
	struct wrapIter mt;
	
	mt._ctl._vptr = &ctl;
	if (!val) {
		mt.it = 0;
		mt.val = "";
		return obj->_vptr->setProperty(obj, name, &mt._ctl);
	} else {
		MPT_INTERFACE(iterator) *it;
		int ret;
		if (!(it = mpt_iterator_string(val, sep))) {
			return MPT_ERROR(BadValue);
		}
		mt.it = it;
		mt.val = val;
		ret = obj->_vptr->setProperty(obj, name, &mt._ctl);
		it->_vptr->ref.unref((void *) it);
		return ret;
	}
}
