/*!
 * delete node subelements.
 */

#include "node.h"

/*!
 * \ingroup mptNode
 * \brief delete subnodes
 * 
 * Remove child elements recursively from node.
 * 
 * \param node  node to remove children
 */
extern void mpt_node_clear(MPT_STRUCT(node) *node)
{
	MPT_STRUCT(node) *tmp = node->children;
	
	/* isolate and destroy children (recursive) */
	while (tmp) {
		MPT_STRUCT(node) *next = tmp->next;
		tmp->next = tmp->prev = tmp->parent = 0;
		(void) mpt_node_destroy(tmp);
		tmp = next;
	}
	node->children = 0;
}

