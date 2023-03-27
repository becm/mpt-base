/*!
 * set metatype data.
 */

#include <string.h>

#include <sys/uio.h>

#include "meta.h"
#include "convert.h"
#include "types.h"

#include "object.h"

struct wrapIter
{
	MPT_INTERFACE(convertable) _ctl;
	MPT_INTERFACE(metatype) *src;
	const char *val, *sep;
};

static int iterConv(MPT_INTERFACE(convertable) *conv, MPT_TYPE(value) type, void *dest)
{
	struct wrapIter *it = (void *) conv;
	if (!type) {
		static const uint8_t fmt[] = { MPT_ENUM(TypeIteratorPtr), 's', 0 };
		if (dest) {
			*((const uint8_t **) dest) = fmt;
			return 0;
		}
		return MPT_ENUM(TypeIteratorPtr);
	}
	if (type == MPT_ENUM(TypeIteratorPtr) || type == MPT_ENUM(TypeMetaPtr)) {
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
			return MPT_metatype_convert(src, type, dest);
		}
		return 's';
	}
	if (type == MPT_type_toVector('c')) {
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
	static const MPT_INTERFACE_VPTR(convertable) ctl = {
		iterConv
	};
	MPT_INTERFACE(metatype) *src;
	struct wrapIter it;
	int ret;
	
	it._ctl._vptr = &ctl;
	it.src = 0;
	it.val = val;
	it.sep = sep;
	ret = obj->_vptr->set_property(obj, name, &it._ctl);
	if ((src = it.src)) {
		src->_vptr->unref(src);
	}
	return ret;
}
