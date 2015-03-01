/*!
 * find node relative to 'node' on position 'pos'.
 * 
 *    0 -> last in list
 *   <0 -> previous #
 *   >0 -> next # (starting at current)
 */

#include <errno.h>

#include "node.h"

/*@null@*/extern MPT_STRUCT(node) *mpt_gnode_pos(const MPT_STRUCT(node) *node, int pos)
{
	if (!node) {
		errno = EFAULT;
		return 0;
	}
	/* get nth predecessor of node */
	if (pos < 0) {
		while (pos++ && node) {
			node = node->prev;
		}
	}
	/* get nth position starting with node */
	else if (pos) {
		while (--pos && node) {
			node = node->next;
		}
	}
	/* pos == 0 -> get last node */
	else {
		while (node->next) {
			node = node->next;
		}
	}
	
	return (MPT_STRUCT(node) *) node;
}

