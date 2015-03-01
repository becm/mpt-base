
#include <errno.h>

#include "node.h"

/*!
 * \ingroup mptNode
 * \~english
 * \brief restore node links
 *
 * use top->bottom/prev->next as reference.
 * can be used after manual concatenation of nodes.
 * 
 * \param node  root node
 */
extern void mpt_gnode_relink(MPT_STRUCT(node) *node)
{
	MPT_STRUCT(node) *start;
	
	if (!(start = node)) {
		errno = EFAULT;
		return;
	}
	
	if (node->children) {
		node->children->parent = node;
	}
	node = node->children;
	
	while (node && node != start) {
		if (node->children) {
			node->children->parent = node;
			if (node->next) {
				node->next->parent = node->parent;
				node->next->prev = node;
			}
			node = node->children;
		}
		if (node->next) {
			node->next->parent = node->parent;
			node->next->prev = node;
			node = node->next;
		}
		else {
			node = node->parent->next;
		}
	}
}
