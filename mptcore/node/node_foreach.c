/*!
 * process node list
 */

#include <errno.h>
#include <string.h>

#include "node.h"

extern const MPT_STRUCT(node) *mpt_node_foreach(const MPT_STRUCT(node) *head, MPT_TYPE(PropertyHandler) proc, void *parg, int mask)
{
	if (!head || !proc) {
		return 0;
	}
	do {
		static const char norm[] = "\0";
		MPT_INTERFACE(metatype) *curr;
		MPT_STRUCT(property) prop;
		int skip;
		
		/* skip masked types */
		skip = head->children ? MPT_ENUM(TraverseNonLeafs) : MPT_ENUM(TraverseLeafs);
		if (mask & skip) {
			continue;
		}
		prop.desc = 0;
		
		/* get current identifier */
		if (mpt_identifier_len(&head->ident) > 0) {
			prop.name = mpt_identifier_data(&head->ident);
		}
		/* avoid empty property */
		else if (mask & MPT_ENUM(TraverseEmpty)) {
			continue;
		}
		else {
			prop.name = norm;
		}
		/* get data from current metatype */
		if ((curr = head->_meta)) {
			/* skip non-default values */
			if (mask & MPT_ENUM(TraverseChange)) {
				continue;
			}
			/* default text metatype */
			if (!curr->_vptr->assign(curr, 0)
			    && (prop.val.ptr = curr->_vptr->typecast(curr, 's'))) {
				prop.val.fmt = 0;
			} else {
				static const char metafmt[2] = { MPT_ENUM(TypeMeta) };
				prop.val.fmt = metafmt;
				prop.val.ptr = &curr;
			}
		}
		/* no property value */
		else {
			if (mask & MPT_ENUM(TraverseDefault)) {
				continue;
			}
			prop.val.fmt = 0;
			prop.val.ptr = 0;
		}
		if (!proc || (skip = proc(parg, &prop)) < 0) {
			return head;
		}
	} while ((head = head->next));
	
	return 0;
}

