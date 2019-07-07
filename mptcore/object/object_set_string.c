/*!
 * set metatype data.
 */

#include <string.h>

#include <sys/uio.h>

#include "convert.h"
#include "meta.h"

#include "object.h"

struct wrapIter
{
	MPT_INTERFACE(metatype) _ctl;
	MPT_INTERFACE(metatype) *src;
	const char *val, *sep;
};

static int metaIterConv(MPT_INTERFACE(convertable) *mt, int type, void *dest)
{
	struct wrapIter *it = (void *) mt;
	if (!type) {
		static const uint8_t fmt[] = { MPT_ENUM(TypeIterator), 's', 0 };
		if (dest) {
			*((const uint8_t **) dest) = fmt;
			return 0;
		}
		return MPT_ENUM(TypeIterator);
	}
	if (type == MPT_type_pointer(MPT_ENUM(TypeIterator))) {
		if (dest) {
			MPT_INTERFACE(metatype) *src;
			if (!it->val || !*it->val) {
				return MPT_ERROR(BadValue);
			}
			if (!(src = it->src)) {
				if (!(src = mpt_iterator_string(it->val, it->sep))) {
					return MPT_ERROR(BadValue);
				}
				it->src = src;
			}
			return MPT_metatype_convert(src, MPT_type_pointer(MPT_ENUM(TypeIterator)), dest);
		}
		return 's';
	}
	if (type == MPT_type_vector('c')) {
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
	if (it->val) {
		int ret = mpt_convert_string(it->val, type, dest);
		if (ret < 0) {
			return ret;
		}
		return 's';
	}
	return 0;
}
static void metaIterUnref(MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
}
static uintptr_t metaIterRef(MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
static MPT_INTERFACE(metatype) *metaIterClone(const MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
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
extern int mpt_object_set_string(MPT_INTERFACE(object) *obj, const char *name, const char *val, const char *sep)
{
	static const MPT_INTERFACE_VPTR(metatype) ctl = {
		{ metaIterConv },
		metaIterUnref,
		metaIterRef,
		metaIterClone
	};
	MPT_INTERFACE(metatype) *src;
	struct wrapIter mt;
	int ret;
	
	mt._ctl._vptr = &ctl;
	mt.src = 0;
	mt.val = val;
	mt.sep = sep;
	ret = obj->_vptr->set_property(obj, name, (MPT_INTERFACE(convertable) *) &mt._ctl);
	if ((src = mt.src)) {
		src->_vptr->unref(src);
	}
	return ret;
}
