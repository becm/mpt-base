/*!
 * log metatype info.
 */

#include "object.h"

#include "meta.h"

/*!
 * \ingroup mptMeta
 * \brief metatype info
 * 
 * Get metatype information.
 * 
 * \param mt  metatype instance
 * \param pr  logger interface
 */
int mpt_meta_info(const MPT_INTERFACE(metatype) *mt, MPT_STRUCT(property) *pr)
{
	const char *desc;
	int code;
	
	code = mt->_vptr->conv(mt, 0, 0);
	
	if (!pr) {
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
		if (mt->_vptr->conv(mt, MPT_ENUM(TypeObject), &obj) >= 0
		    && obj
		    && (obj->_vptr->property(obj, pr) >= 0)
		    && pr->name) {
			pr->desc = pr->name;
			pr->name = MPT_tr("object");
		} else {
			pr->desc = desc;
			pr->name = MPT_tr("interface");
		}
		return code;
	}
	/* generic instance */
	mt->_vptr->conv(mt, MPT_ENUM(TypeValue), &pr->val);
	
	/* output typecode */
	if ((desc = mpt_meta_typename(code)) && !*desc) {
		desc = 0;
	}
	pr->name = MPT_tr("metatype");
	pr->desc = desc;
	
	return code;
}
