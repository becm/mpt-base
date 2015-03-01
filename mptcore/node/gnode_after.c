/*!
 * insert node after supplied.
 */

#include <errno.h>

#include "node.h"

extern MPT_STRUCT(node) *mpt_gnode_after(MPT_STRUCT(node) *position, MPT_STRUCT(node) *insert)
{
	if (!insert) {
		errno = EFAULT;
		return 0;
	}
	
	/* no previous node, avoid self-reference */
	if (!position || insert == position) {
		return insert;
	}
	/* avoid self-reference */
	insert->prev = position;
	insert->next = position->next;
	position->next = insert;
	
	insert->parent = position->parent;
	
	if ((position = insert->next)) {
		position->prev = insert;
	}
	return insert;
}

