/*!
 * list/tree copy operations.
 *
 * option to add reference copy data.
 */

#include <errno.h>

#include "node.h"

static MPT_STRUCT(node) *list_copy(const MPT_STRUCT(node) *src, MPT_STRUCT(node) *last, MPT_STRUCT(node) *(*cpy)(const MPT_STRUCT(node) *, MPT_STRUCT(node) *))
{
	MPT_STRUCT(node) *first = 0;
	
	for ( ; src ; src = src->next) {
		MPT_STRUCT(node) *dest = cpy(src, 0);
		
		if (dest) last = mpt_gnode_after(last, dest);
		
		if (!first) first = dest;
	}
	return first;
}

extern MPT_STRUCT(node) *mpt_list_clone(const MPT_STRUCT(node) *src, MPT_STRUCT(node) *after)
{
	return list_copy(src, after, mpt_node_clone);
}
extern MPT_STRUCT(node) *mpt_list_copy(const MPT_STRUCT(node) *src, MPT_STRUCT(node) *after)
{
	return list_copy(src, after, mpt_node_copy);
}

static MPT_STRUCT(node) *tree_copy(const MPT_STRUCT(node) *src, MPT_STRUCT(node) *(*cpy)(const MPT_STRUCT(node) *, MPT_STRUCT(node) *))
{
	MPT_STRUCT(node) first, *ret;
	
	if (!src || !(ret = mpt_node_clone(src, 0))) return 0;
	
	if (!src->children) return ret;
	
	first.next = first.prev = first.children = 0;
	first.parent = ret;
	
	ret->children = list_copy(src->children, &first, cpy);
	
	return ret;
}
extern MPT_STRUCT(node) *mpt_tree_clone(const MPT_STRUCT(node) *src)
{
	return tree_copy(src, (MPT_STRUCT(node) *(*)()) mpt_tree_clone);
}

extern MPT_STRUCT(node) *mpt_tree_copy(const MPT_STRUCT(node) *src)
{
	return tree_copy(src, (MPT_STRUCT(node) *(*)()) mpt_tree_copy);
}
