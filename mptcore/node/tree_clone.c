/*!
 * list/tree copy operations.
 *
 * option to add reference copy data.
 */

#include <errno.h>

#include "node.h"

extern MPT_STRUCT(node) *mpt_list_clone(const MPT_STRUCT(node) *src)
{
	MPT_STRUCT(node) *first = 0, *last = 0;
	
	for (; src; src = src->next) {
		MPT_STRUCT(node) *cpy;
		
		if ((cpy = mpt_node_clone(src))) {
			if (!last) {
				last = first = cpy;
			} else {
				last = mpt_gnode_after(last, cpy);
			}
			/* require empty or cloned subtree */
			if (!src->children
			    || !(cpy->children = mpt_list_clone(src->children))) {
				continue;
			}
		}
		for (cpy = first; cpy; cpy = first) {
			first = first->next;
			mpt_node_unlink(cpy);
			mpt_node_destroy(cpy);
		}
		return 0;
	}
	return first;
}
extern MPT_STRUCT(node) *mpt_tree_clone(const MPT_STRUCT(node) *src)
{
	MPT_STRUCT(node) *cpy;
	
	if (!(cpy = mpt_node_clone(src))) {
		return 0;
	}
	if (src->children
	    && !(cpy->children = mpt_list_clone(src->children))) {
		mpt_node_destroy(cpy);
		return 0;
	}
	return cpy;
}
