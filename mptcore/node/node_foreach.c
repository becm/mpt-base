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
		static const char norm[] = "\0", *name;
		MPT_INTERFACE(metatype) *curr;
		MPT_STRUCT(property) prop;
		int skip;
		
		/* skip masked types */
		skip = head->children ? MPT_ENUM(TraverseNonLeafs) : MPT_ENUM(TraverseLeafs);
		if (mask & skip) {
			continue;
		}
		prop.desc = 0;
		prop.val.fmt = 0;
		prop.val.ptr = 0;
		
		/* get current identifier */
		if (!(name = mpt_identifier_data(&head->ident, 0))) {
			static const char fmt[] = { MPT_ENUM(TypeNode), 0 };
			if (mask & MPT_ENUM(TraverseEmpty)) {
				continue;
			}
			prop.name = 0;
			prop.val.fmt = fmt;
			prop.val.ptr = (void *) head;
			if (!proc || (skip = proc(parg, &prop)) < 0) {
				return head;
			}
			if (skip) {
				continue;
			}
		}
		/* get data from current metatype */
		if ((curr = head->_meta)) {
			prop.name = norm;
			skip = curr->_vptr->property(curr, &prop, 0);
			
			if (skip < 0 || !prop.val.ptr || (prop.val.fmt && !*prop.val.fmt)) {
				prop.val.ptr = curr->_vptr->typecast(curr, 's');
				prop.val.fmt  = 0;
			}
			else if (!skip && (mask & MPT_ENUM(TraverseDefault))) {
				continue;
			}
		}
		prop.name = name;
		
		/* get data from current metatype */
		if (!prop.val.ptr) {
			if (mask & MPT_ENUM(TraverseDefault)) {
				continue;
			}
			if (!proc || (skip = proc(parg, &prop)) < 0) {
				return head;
			}
			if (skip) {
				continue;
			}
			prop.val.ptr = norm;
		}
		/* set property data */
		if (mask & MPT_ENUM(TraverseChange)) {
			continue;
		}
		if ((skip = proc(parg, &prop)) < 0) {
			return head;
		}
	} while ((head = head->next));
	
	return 0;
}

