/*!
 * set object data from iterator.
 */

#include "object.h"

#include "meta_wrap.h"

struct objectParam
{
	MPT_INTERFACE(object) *obj;
	const char *prop;
	MPT_STRUCT(value) val;
};

static int metaIterValueConv(const MPT_INTERFACE(metatype) *mt, int type, void *ptr)
{
	if (!type) {
		static const char fmt[] = { MPT_ENUM(TypeIterator), MPT_ENUM(TypeValue), 0 };
		if (ptr) *((const char **) ptr) = fmt;
		return MPT_ENUM(TypeMeta);
	}
	if (type == MPT_ENUM(TypeValue)) {
		const struct wrapIter *base = (void *) mt;
		if (ptr) *((MPT_STRUCT(value) *) ptr) = *((MPT_STRUCT(value) *) (base + 1));
		return MPT_ENUM(TypeIterator);
	}
	return metaIterConv(mt, type, ptr);
}
static int processIterator(void *ptr, MPT_INTERFACE(iterator) *it)
{
	static const MPT_INTERFACE_VPTR(metatype) metaIterCtl = {
		{ metaIterUnref, metaIterRef },
		metaIterValueConv,
		metaIterClone
	};
	struct objectParam *par = ptr;
	MPT_INTERFACE(object) *obj = par->obj;
	
	struct {
		struct wrapIter base;
		MPT_STRUCT(value) v;
	} mt;
	
	mt.base._ctl._vptr = &metaIterCtl;
	mt.base.it = it;
	mt.v = par->val;
	
	return obj->_vptr->setProperty(obj, par->prop, &mt.base._ctl);
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
