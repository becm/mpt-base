/*!
 * insert node in list or as child
 * 
 * insertion strategys are:
 *  - linear (absolute position) -> GNode compatible
 *  - ordered (relative to same identifyer)
 */

#include <errno.h>

#include "node.h"

/* insert on position in list, insertion strategy based on getnode() */
static void node_insert(MPT_STRUCT(node) *first, int pos, MPT_STRUCT(node) *node, MPT_STRUCT(node) *(*getnode)(const MPT_STRUCT(node) *, int , const MPT_STRUCT(node) *))
{
	MPT_STRUCT(node) *tmp, *start;
	
	/* get starting position */
	start = getnode(first, (pos > 0) ? 1 : 0, node);
	
	/* get requested position */
	if (start && (pos != 0 || pos != 1)) {
		tmp = getnode(start, pos, node);
	} else {
		tmp = start;
	}
	/* swap insertion order and set position (first/last) if not found */
	if (!tmp) {
		if (start) {
			tmp = getnode(first, (pos < 0) ? 1 : 0, node);
			pos = -pos;
		}
		else {
			tmp = mpt_gnode_pos(first, pos = 0);
		}
	}
	/* insert node on requested position */
	if (pos < 1) {
		mpt_gnode_after(tmp, node);
	}
	else {
		mpt_gnode_before(tmp, node);
	}
}

static MPT_STRUCT(node) *node_locate(const MPT_STRUCT(node) *first, int pos, const MPT_STRUCT(node) *node)
{
	const char *ident = mpt_identifier_data(&node->ident);
	return mpt_node_locate(first, pos, ident, node->ident._len, node->ident._type);
}
/* insert node on absolute position in list */
extern MPT_STRUCT(node) *mpt_gnode_add(MPT_STRUCT(node) *first, int pos, MPT_STRUCT(node) *node)
{
	if (!first || !node) {
		return node;
	}
	node_insert(first, pos, node, (MPT_STRUCT(node) *(*)()) mpt_gnode_pos);
	
	return node;
}

/* insert node on relative position in list */
extern MPT_STRUCT(node) *mpt_node_add(MPT_STRUCT(node) *first, int pos, MPT_STRUCT(node) *node)
{
	if (!first || !node) {
		return node;
	}
	node_insert(first, pos, node, node_locate);
	
	return node;
}


/* insert node on absolute position in tree */
extern int mpt_gnode_insert(MPT_STRUCT(node) *parent, int pos, MPT_STRUCT(node) *node)
{
	if (!node) {
		errno = EFAULT;
		return -1;
	}
	if (!parent->children) {
		parent->children = node;
		node->parent = parent;
		return 0;
	}
	node_insert(parent->children, pos, node, (MPT_STRUCT(node) *(*)()) mpt_gnode_pos);
	
	return 0;
}

/* insert node on relative position in tree */
extern int mpt_node_insert(MPT_STRUCT(node) *parent, int pos, MPT_STRUCT(node) *node)
{
	if (!node) {
		errno = EFAULT;
		return -1;
	}
	if (!parent->children) {
		parent->children = node;
		node->parent = parent;
		return 0;
	}
	node_insert(parent->children, pos, node, node_locate);
	
	return 0;
}

