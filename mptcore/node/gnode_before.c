/*!
 * insert node before supplied.
 */

#include <errno.h>

#include "node.h"

extern MPT_STRUCT(node) *mpt_gnode_before(MPT_STRUCT(node) *position, MPT_STRUCT(node) *insert)
{
	if ( !insert ) {
		errno = EFAULT; return 0;
	}
	
	/* no previous node, avoid self-reference */
	if ( !position || insert == position )
		return insert;
	
	/* avoid self-reference */
	insert->prev = position->prev;
	insert->next = position;
	position->prev = insert;
	
	insert->parent = position->parent;
	
	if ( (position = insert->prev) )
		position->next = insert;
	else if ( (position = insert->parent) )
		position->children = insert;
	
	/* return inserted node */
	return insert;
}

