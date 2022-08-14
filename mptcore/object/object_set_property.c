/*!
 * process node list
 */

#include <string.h>

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
extern int mpt_object_set_property(MPT_INTERFACE(object) *obj, int match, const MPT_STRUCT(identifier) *id, MPT_INTERFACE(convertable) *val)
{
	const char *name = 0;
	int ret;
	
	/* get current identifier */
	if (id && id->_len) {
		if (id->_charset != MPT_CHARSET(UTF8)) {
			return MPT_ERROR(BadEncoding);
		}
		if (!(name = mpt_identifier_data(id))) {
			return MPT_ERROR(MissingData);
		}
	}
	/* no global property */
	if (!name && !(match & MPT_ENUM(TraverseEmpty))) {
		return MPT_ENUM(TraverseEmpty);
	}
	/* get data from current metatype */
	if (val) {
		const char *str;
		/* skip non-default values */
		if (!(match & MPT_ENUM(TraverseChange))) {
			return MPT_ENUM(TraverseChange);
		}
		/* use text parser for string content */
		if ((ret = val->_vptr->convert(val, 's', &str)) >= 0) {
			ret = mpt_object_set_string(obj, name, ret ? str : 0, 0);
		} else {
			ret = obj->_vptr->set_property(obj, name, val);
		}
	}
	/* no property value */
	else {
		if (!(match & MPT_ENUM(TraverseDefault))) {
			return MPT_ENUM(TraverseDefault);
		}
		ret = obj->_vptr->set_property(obj, name, 0);
	}
	if (ret == MPT_ERROR(BadArgument)) {
		if (!(match & MPT_ENUM(TraverseUnknown))) {
			return ret;
		}
		return MPT_ENUM(TraverseUnknown);
	}
	return ret < 0 ? ret : 0;
}

