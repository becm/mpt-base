/*!
 * log metatype info.
 */

#include <string.h>

#include "types.h"

#include "object.h"

/*!
 * \ingroup mptCore
 * \brief metatype info
 * 
 * Fill property with convertable instance description.
 * 
 * \param val metatype instance
 * \param pr  property data
 */
int mpt_convertable_info(MPT_INTERFACE(convertable) *val, MPT_STRUCT(property) *pr)
{
	MPT_INTERFACE(object) *obj = 0;
	const MPT_STRUCT(named_traits) *traits;
	const char *desc;
	int code;
	
	code = val->_vptr->convert(val, 0, 0);
	
	if ((code < 0) || !pr) {
		return code;
	}
	/* object specific name */
	if (val->_vptr->convert(val, MPT_ENUM(TypeObjectPtr), &obj) >= 0
	 && obj) {
		pr->name = "";
		pr->desc = 0;
		if ((obj->_vptr->property(obj, pr) >= 0)) {
			pr->desc = pr->name;
			pr->name = "object";
			return code;
		}
	}
	/* interface instance */
	desc = pr->desc;
	if ((traits = mpt_interface_traits(code))) {
		pr->name = "interface";
		pr->desc = traits->name;
	}
	/* metatype instance */
	else if ((traits = mpt_metatype_traits(code))) {
		pr->name = "metatype";
		pr->desc = traits->name;
	}
	/* generic convertable */
	else {
		pr->name = "convertable";
		pr->desc = 0;
	}
	/* generic value data */
	if (val->_vptr->convert(val, MPT_ENUM(TypeValue), &pr->val) < 0) {
		MPT_property_set_string(pr, desc);
	}
	return code;
}
