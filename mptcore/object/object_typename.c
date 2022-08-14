/*!
 * MPT core library
 *   treat default property name as object type.
 */

#include "types.h"

#include "object.h"

/* get metatype name */
/*!
 * \ingroup mptObject
 * \brief object name
 * 
 * Get name of default property.
 * This should be equivalent to the type represented by an object.
 * 
 * \param mt  MPT object interface pointer
 * 
 * \return name of object type
 */
extern const char *mpt_object_typename(MPT_INTERFACE(object) *mt)
{
	MPT_STRUCT(property) pr = MPT_PROPERTY_INIT;
	
	pr.name = "";
	if (mt->_vptr->property(mt, &pr) < 0) {
		return 0;
	}
	return pr.name;
}
