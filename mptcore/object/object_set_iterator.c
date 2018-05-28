/*!
 * set object data from iterator.
 */

#include "object.h"

#include "meta_wrap.h"

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
extern int mpt_object_set_iterator(MPT_INTERFACE(object) *obj, const char *prop, MPT_INTERFACE(iterator) *it)
{
	static const MPT_INTERFACE_VPTR(metatype) metaIterCtl = {
		{ metaIterUnref, metaIterRef },
		metaIterConv,
		metaIterClone
	};
	struct wrapIter mt;
	
	mt._ctl._vptr = &metaIterCtl;
	mt.it = it;
	
	return obj->_vptr->property_set(obj, prop, &mt._ctl);
}
