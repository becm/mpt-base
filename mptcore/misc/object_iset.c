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
extern int mpt_object_iset(MPT_INTERFACE(object) *obj, const char *prop, MPT_INTERFACE(iterator) *it)
{
	struct wrapIter mt;
	
	mt._ctl._vptr = &metaIterCtl;
	mt.it = it;
	
	return obj->_vptr->setProperty(obj, prop, &mt._ctl);
}
