/*!
 * process node list
 */

#include <string.h>

#include "meta.h"

#include "object.h"

/*!
 * \ingroup mptObject
 * \brief set object properties
 * 
 * Set object properties from node list.
 * 
 * \param obj    object interface descriptor
 * \param match  traverse flags
 * \param id     property identifier
 * \param val    new property value
 * 
 * \return error code, skip reason or success
 */
extern int mpt_object_set_property(MPT_INTERFACE(object) *obj, int match, const MPT_STRUCT(identifier) *id, const MPT_INTERFACE(metatype) *val)
{
	const char *name;
	int ret;
	
	/* get current identifier */
	if (id && mpt_identifier_len(id) > 0) {
		if (id->_type) {
			return MPT_ERROR(BadEncoding);
		}
		if (!(name = mpt_identifier_data(id))) {
			return MPT_ERROR(MissingData);
		}
	} else {
		/* avoid empty property */
		if (!(match & MPT_ENUM(TraverseEmpty))) {
			return MPT_ENUM(TraverseEmpty);
		}
		name = 0;
	}
	/* get data from current metatype */
	if (val) {
		const char *str;
		/* skip non-default values */
		if (!(match & MPT_ENUM(TraverseChange))) {
			return MPT_ENUM(TraverseChange);
		}
		/* use text parser for string content */
		if ((ret = val->_vptr->conv(val, 's', &str)) >= 0) {
			ret = mpt_object_set_string(obj, name, ret ? str : 0, 0);
		} else {
			ret = obj->_vptr->property_set(obj, name, val);
		}
	}
	/* no property value */
	else {
		if (!(match & MPT_ENUM(TraverseDefault))) {
			return MPT_ENUM(TraverseDefault);
		}
		ret = obj->_vptr->property_set(obj, name, 0);
	}
	if (ret == MPT_ERROR(BadArgument)) {
		if (!(match & MPT_ENUM(TraverseUnknown))) {
			return ret;
		}
		return MPT_ENUM(TraverseUnknown);
	}
	return ret < 0 ? ret : 0;
}

