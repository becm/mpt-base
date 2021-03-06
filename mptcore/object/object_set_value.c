/*!
 * set object data from iterator.
 */

#include "object.h"
#include "convert.h"
#include "types.h"

#include "meta.h"

#include "iterator_convert.h"

struct objectParam
{
	MPT_INTERFACE(object) *obj;
	const char *prop;
	MPT_STRUCT(value) val;
};

static int iteratorValueConv(MPT_INTERFACE(convertable) *conv, int type, void *ptr)
{
	const struct convIter *base = (void *) conv;
	const MPT_STRUCT(value) *val = (void *) (base + 1);
	int ret;
	
	if (!type) {
		static const uint8_t fmt[] = { MPT_ENUM(TypeIteratorPtr), MPT_ENUM(TypeValue), 0 };
		if (ptr) {
			*((const uint8_t **) ptr) = fmt;
			return 0;
		}
		return MPT_ENUM(TypeIteratorPtr);
	}
	if (type == MPT_ENUM(TypeValue)) {
		if (ptr) *((MPT_STRUCT(value) *) ptr) = *val;
		return MPT_ENUM(TypeIteratorPtr);
	}
	if ((ret = iteratorConv(conv, type, ptr)) >= 0) {
		return ret;
	}
	if (!val->fmt || !val->ptr || !val->fmt[0] || val->fmt[1]) {
		return MPT_ERROR(BadValue);
	} else {
		const void *from = val->ptr;
		return mpt_data_convert(&from, val->fmt[0], ptr, type);
	}
}
static int processIterator(void *ptr, MPT_INTERFACE(iterator) *it)
{
	static const MPT_INTERFACE_VPTR(convertable) convIterCtl = {
		iteratorValueConv
	};
	struct objectParam *par = ptr;
	MPT_INTERFACE(object) *obj = par->obj;
	
	struct {
		struct convIter base;
		MPT_STRUCT(value) v;
	} conv;
	
	conv.base._ctl._vptr = &convIterCtl;
	conv.base.it = it;
	conv.v = par->val;
	
	return obj->_vptr->set_property(obj, par->prop, &conv.base._ctl);
}
/*!
 * \ingroup mptObject
 * \brief set object property
 * 
 * Set property data to value.
 * Use iterator as intermediate for type conversions.
 * 
 * \param obj  object interface descriptor
 * \param prop name of property to change
 * \param val  data to set
 */
extern int mpt_object_set_value(MPT_INTERFACE(object) *obj, const char *prop, MPT_STRUCT(value) *val)
{
	
	struct objectParam par;
	
	if (!(par.obj = obj)) {
		return MPT_ERROR(BadArgument);
	}
	par.prop = prop;
	if (val) {
		par.val = *val;
	} else {
		par.val.fmt = 0;
		par.val.ptr = 0;
	}
	return mpt_process_value(val, processIterator, &par);
}
