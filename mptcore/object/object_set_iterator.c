/*!
 * set object data from iterator.
 */

#include "object.h"

#include "iterator_convert.h"

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
	static const MPT_INTERFACE_VPTR(convertable) convIterCtl = {
		iteratorConv
	};
	struct convIter conv;
	
	conv._ctl._vptr = &convIterCtl;
	conv.it = it;
	
	return obj->_vptr->set_property(obj, prop, &conv._ctl);
}
