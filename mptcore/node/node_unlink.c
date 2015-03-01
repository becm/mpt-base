
#include <string.h>
#include <errno.h>

#include "node.h"

/*!
 * \ingroup mptNode
 * \~english
 * \brief unlink node
 * 
 * Remove references to and from MPT node.
 * 
 * \param curr  node to unlink
 */
/*@null@*/extern MPT_STRUCT(node) *mpt_node_unlink(MPT_STRUCT(node) *curr)
{
	MPT_STRUCT(node)  *next;
	
	if (!curr) {
		errno = EINVAL;
		return 0;
	}
	/* change predecessor of successor */
	if ((next = curr->next)) {
		next->prev = curr->prev;
	}
	/* change successor of predecessor */
	if (curr->prev) {
		curr->prev->next = next;
	}
	/* if no predecessor change child link of parent (if exists) */
	else if (curr->parent) {
		curr->parent->children = next;
	}
	curr->parent = curr->next = curr->prev = 0;
	
	return next;
}

