/*!
 * set object data from iterator.
 */

#include "object.h"

#include "meta_wrap.h"

struct objectParam
{
	MPT_INTERFACE(object) *obj;
	const char *prop;
};
static int processIterator(void *ptr, MPT_INTERFACE(iterator) *it)
{
	struct objectParam *par = ptr;
	MPT_INTERFACE(object) *obj = par->obj;
	struct wrapIter mt;
	
	mt._ctl._vptr = &metaIterCtl;
	mt.it = it;
	
	return obj->_vptr->setProperty(obj, par->prop, &mt._ctl);
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
	
	return mpt_process_value(val, processIterator, &par);
}
