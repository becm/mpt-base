/*!
 * process node list
 */

#include <string.h>

#include "meta.h"
#include "node.h"

#include "object.h"

/*!
 * \ingroup mptObject
 * \brief set object properties
 * 
 * Set object properties from node list.
 * 
 * \param obj    object interface descriptor
 * \param node   data node
 * \param match  traverse flags
 */
extern int mpt_object_set_node(MPT_INTERFACE(object) *obj, const MPT_STRUCT(node) *val, int match)
{
	const MPT_INTERFACE(metatype) *curr;
	const char *name;
	int flag, ret;
	
	/* skip masked types */
	flag = val->children ? MPT_ENUM(TraverseNonLeafs) : MPT_ENUM(TraverseLeafs);
	if (!(flag & match)) {
		return 0;
	}
	/* get current identifier */
	if (mpt_identifier_len(&val->ident) > 0) {
		name = mpt_identifier_data(&val->ident);
	} else {
		/* avoid empty property */
		if (!(match & MPT_ENUM(TraverseEmpty))) {
			return 0;
		}
		/* allow only single base assignment */
		match &= MPT_ENUM(TraverseEmpty);
		name = 0;
	}
	/* get data from current metatype */
	if ((curr = val->_meta)) {
		const char *str;
		/* skip non-default values */
		if (!(match & MPT_ENUM(TraverseChange))) {
			return 0;
		}
		/* use text parser for string content */
		if ((ret = curr->_vptr->conv(curr, 's', &str)) >= 0) {
			ret = mpt_object_set_string(obj, name, ret ? str : 0, 0);
		} else {
			ret = obj->_vptr->setProperty(obj, name, curr);
		}
	}
	/* no property value */
	else {
		if (!(match & MPT_ENUM(TraverseDefault))) {
			return 0;
		}
		ret = obj->_vptr->setProperty(obj, name, 0);
	}
	if (ret == MPT_ERROR(BadArgument)) {
		if (!(flag & MPT_ENUM(TraverseUnknown))) {
			return ret;
		}
		return 0;
	}
	return ret;
}

