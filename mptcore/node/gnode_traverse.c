/*!
 * MPT node traversal
 *   implementation of different traversion types
 */

#include <errno.h>

#include "node.h"

/* set traversion state according to flags */
#define MPT_traverse_curr(c,fl)  ((c)->children ? ((fl) & MPT_ENUM(TraverseNonLeafs)) : ((fl) & MPT_ENUM(TraverseLeafs)))


/*!
 * \ingroup mptNode
 * \brief traverse tree in-order
 * 
 * traversion sequence:
 *  - first child
 *  - self
 *  - other children
 * 
 * \param node     top node element
 * \param flags    traversion flags
 * \param depth    current traversion depth
 * \param traverse traverse function
 * \param data     argument for traverse function
 * 
 * \return node for which traverse function returned non-zero
 */
extern MPT_STRUCT(node) *_mpt_gnode_traverse_in(MPT_STRUCT(node) *node, int flags, size_t depth, MPT_TYPE(node_handler) traverse, void *data)
{
	MPT_STRUCT(node) *child, *tmp;
	
	/* process first child before current */
	if ((child = node->children)) {
		tmp = _mpt_gnode_traverse_in(child, flags, depth+1, traverse, data);
		if (tmp) {
			return tmp;
		}
		child = child->next;
	}
	/* process current node */
	if (MPT_traverse_curr(node, flags)) {
		if (traverse(node, data, depth)) {
			return node;
		}
	}
	/* process (remaining) children */
	while (child) {
		tmp = _mpt_gnode_traverse_in(child, flags, depth+1, traverse, data);
		if (tmp) {
			return tmp;
		}
		child = child->next;
	}
	return 0;
}

/*!
 * \ingroup mptNode
 * \brief traverse tree levels
 * 
 * traversion sequence:
 *  - all nodes in current list
 *  - nodes in sublevel(s)
 * 
 * \param node     top node element
 * \param flags    traversion flags
 * \param depth    current traversion depth
 * \param traverse traverse function
 * \param data     argument for traverse function
 * 
 * \return node for which traverse function returned non-zero
 */
extern MPT_STRUCT(node) *_mpt_gnode_traverse_level(MPT_STRUCT(node) *node, int flags, size_t depth, MPT_TYPE(node_handler) traverse, void *data)
{
	MPT_STRUCT(node) *curr = node;
	size_t up = 0;
	
	while (node) {
		/* maximal parent levels to search for further nodes on current level */
		while (curr) {
			/* process current node */
			if (MPT_traverse_curr(curr, flags)) {
				if (traverse(curr, data, depth)) {
					return curr;
				}
			}
			/* find next node in current level if current has no successor */
			if (!curr->next) curr = mpt_gnode_samelevel(curr, up);
			else curr = curr->next;
		}
		/* get first node in next sublevel */
		curr = node = mpt_gnode_sublevel(node, up++);
		++depth;
	}
	return 0;
}

/*!
 * \ingroup mptNode
 * \brief traverse tree in post-order
 * 
 *  traversion sequence:
 *   - all children
 *   - self
 * 
 * \param node     top node element
 * \param flags    traversion flags
 * \param depth    current traversion depth
 * \param traverse traverse function
 * \param data     argument for traverse function
 * 
 * \return node for which traverse function returned non-zero
 */
extern MPT_STRUCT(node) *_mpt_gnode_traverse_post(MPT_STRUCT(node) *node, int flags, size_t depth, MPT_TYPE(node_handler) traverse, void *data)
{
	MPT_STRUCT(node) *child, *tmp;
	
	child = node->children;
	
	/* process children */
	while (child) {
		tmp = _mpt_gnode_traverse_post(child, flags, depth+1, traverse, data);
		if (tmp) {
			return tmp;
		}
		child = child->next;
	}
	/* process current node */
	if (MPT_traverse_curr(node, flags)) {
		if (traverse(node, data, depth)) {
			return node;
		}
	}
	return 0;
}

/*!
 * \ingroup mptNode
 * \brief traverse tree in pre-order
 * 
 * traversion sequence:
 *  - self
 *  - all children
 * 
 * \param node     top node element
 * \param flags    traversion flags
 * \param depth    current traversion depth
 * \param traverse traverse function
 * \param data     argument for traverse function
 * 
 * \return node for which traverse function returned non-zero
 */
extern MPT_STRUCT(node) *_mpt_gnode_traverse_pre(MPT_STRUCT(node) *node, int flags, size_t depth, MPT_TYPE(node_handler) traverse, void *data)
{
	MPT_STRUCT(node) *tmp;
	
	/* process current node */
	if (MPT_traverse_curr(node, flags)) {
		if (traverse(node, data, depth)) return node;
	}
	
	/* process first child before current */
	if (!(node = node->children)) return 0;
	
	/* process (remaining) children */
	do {
		tmp = _mpt_gnode_traverse_pre(node, flags, depth+1, traverse, data);
		if (tmp) {
			return tmp;
		}
	} while ((node = node->next));
	
	return 0;
}

/*!
 * \ingroup mptNode
 * \brief traverse node list
 * 
 * call traverse function for nodes and subnodes in node list
 * 
 * \param node     top node element
 * \param flags    traversion flags
 *   TraverseLeafs:      process only leaf nodes
 *   TraverseNonLeafs:   process only non-leaf nodes
 *   TraverseAll:        process both node types
 *   TraverseInOrder:    first child, node, other children
 *   TraversePreOrder:   node, children
 *   TraversePostOrder:  children, node
 *   TraverseLevelOrder: nodes on current, then on sub-level(s)
 * \param traverse traverse function
 * \param data     argument for traverse function
 * 
 * \return node for which traverse function returned non-zero
 */
extern MPT_STRUCT(node) *mpt_gnode_traverse(MPT_STRUCT(node) *node, int flags, MPT_TYPE(node_handler) traverse, void *data)
{
	MPT_STRUCT(node) *(*process)(MPT_STRUCT(node) *, int, size_t, MPT_TYPE(node_handler), void *);
	
	if (!node || !traverse) {
		errno = EFAULT;
		return 0;
	}
	
	switch (flags & MPT_ENUM(TraverseOrders)) {
		case MPT_ENUM(TraverseInOrder):
			process = _mpt_gnode_traverse_in; break;
		case MPT_ENUM(TraversePreOrder):
			process = _mpt_gnode_traverse_pre; break;
		case MPT_ENUM(TraversePostOrder):
			process = _mpt_gnode_traverse_post; break;
		case MPT_ENUM(TraverseLevelOrder):
			return _mpt_gnode_traverse_level(node, flags, 0, traverse, data);
		default:
			errno = EINVAL;
			return 0;
	}
	do {
		MPT_STRUCT(node) *ret = process(node, flags, 0, traverse, data);
		if (ret) {
			return ret;
		}
	} while ((node = node->next));
	
	return 0;
}



