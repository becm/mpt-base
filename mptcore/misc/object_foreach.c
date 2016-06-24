/*!
 * call function for all object properties
 */

#include "object.h"

static int getProperty(void *ptr, MPT_STRUCT(property) *pr)
{
	MPT_INTERFACE(object) *obj = ptr;
	return obj->_vptr->property(obj, pr);
}

/*!
 * \ingroup mptObject
 * \brief process properties
 * 
 * Process object properties matching to mask.
 * 
 * \param obj  property source
 * \param proc process retreived properties
 * \param data argumernt for property processing
 * \param mask properties to process
 * 
 * \return index of requested property
 */
extern int mpt_object_foreach(const MPT_INTERFACE(object) *obj, MPT_TYPE(PropertyHandler) proc, void *data, int mask)
{
	return mpt_generic_foreach(getProperty, (void *) obj, proc, data, mask);
}
