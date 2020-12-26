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
	const char *desc;
	int code;
	
	code = val->_vptr->convert(val, 0, 0);
	
	if ((code < 0) || !pr) {
		return code;
	}
	pr->val.fmt = 0;
	pr->val.ptr = 0;
	
	/* interface instance */
	if ((desc = mpt_interface_typename(code))) {
		MPT_INTERFACE(object) *obj = 0;
		
		if (!*desc) {
			desc = 0;
		}
		pr->name = "";
		pr->desc = 0;
		/* object specific name */
		if (val->_vptr->convert(val, MPT_ENUM(TypeObjectPtr), &obj) >= 0
		    && obj
		    && (obj->_vptr->property(obj, pr) >= 0)
		    && pr->name) {
			pr->desc = pr->name;
			pr->name = MPT_tr("object");
		} else {
			pr->desc = desc;
			pr->name = MPT_tr("interface");
			val->_vptr->convert(val, MPT_ENUM(TypeValue), &pr->val);
		}
		return code;
	}
	/* generic instance */
	val->_vptr->convert(val, MPT_ENUM(TypeValue), &pr->val);
	
	/* output typecode */
	if ((desc = mpt_meta_typename(code)) && !*desc) {
		desc = 0;
	}
	pr->name = MPT_tr("metatype");
	pr->desc = desc;
	
	return code;
}
