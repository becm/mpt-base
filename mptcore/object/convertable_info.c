/*!
 * log metatype info.
 */

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
	const MPT_STRUCT(named_traits) *traits;
	int code;
	
	code = val->_vptr->convert(val, 0, 0);
	
	if ((code < 0) || !pr) {
		return code;
	}
	pr->val.fmt = 0;
	pr->val.ptr = 0;
	
	/* interface instance */
	if ((traits = mpt_interface_traits(code))) {
		MPT_INTERFACE(object) *obj = 0;
		
		pr->name = "";
		pr->desc = 0;
		/* object specific name */
		if (val->_vptr->convert(val, MPT_ENUM(TypeObjectPtr), &obj) >= 0
		    && obj
		    && (obj->_vptr->property(obj, pr) >= 0)
		    && pr->name) {
			pr->desc = pr->name;
			pr->name = "object";
		} else {
			pr->desc = traits->name;
			pr->name = "interface";
			val->_vptr->convert(val, MPT_ENUM(TypeValue), &pr->val);
		}
		return code;
	}
	/* generic instance */
	val->_vptr->convert(val, MPT_ENUM(TypeValue), &pr->val);
	
	/* output typecode */
	if ((traits = mpt_metatype_traits(code))) {
		pr->name = "metatype";
		pr->desc = traits->name;
		return code;
	}
	pr->name = "convertable";
	pr->desc = 0;
	
	return code;
}
