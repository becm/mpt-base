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
 * \param head   list base node
 * \param match  traverse flags
 */
extern const MPT_STRUCT(node) *mpt_object_set_list(MPT_INTERFACE(object) *obj, const MPT_STRUCT(node) *head, int match)
{
	if (!head) {
		return 0;
	}
	do {
		const MPT_INTERFACE(metatype) *curr;
		const char *name;
		int flag, ret;
		
		/* skip masked types */
		flag = head->children ? MPT_ENUM(TraverseNonLeafs) : MPT_ENUM(TraverseLeafs);
		if (!(flag & match)) {
			continue;
		}
		
		/* get current identifier */
		if (mpt_identifier_len(&head->ident) > 0) {
			name = mpt_identifier_data(&head->ident);
		} else {
			/* avoid empty property */
			if (!(match & MPT_ENUM(TraverseEmpty))) {
				continue;
			}
			/* allow only single base assignment */
			match &= MPT_ENUM(TraverseEmpty);
			name = 0;
		}
		/* get data from current metatype */
		if ((curr = head->_meta)) {
			const char *str;
			/* skip non-default values */
			if (!(match & MPT_ENUM(TraverseChange))) {
				continue;
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
				continue;
			}
			ret = obj->_vptr->setProperty(obj, name, 0);
		}
		if (ret == MPT_ERROR(BadArgument)) {
			if (!(flag & MPT_ENUM(TraverseUnknown))) {
				return head;
			}
			continue;
		}
	} while ((head = head->next));
	
	return 0;
}

