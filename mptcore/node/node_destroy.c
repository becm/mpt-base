/*!
 * node destruction
 */

#include <stdlib.h>
#include <errno.h>

#include "node.h"

/*!
 * \ingroup mptNode
 * \brief destroy node
 * 
 * Check if node is accessible from same or upper level,
 * clear subelements before delete.
 * 
 * \param node  node pointer to free
 * 
 * \return node pointer if still enclosed
 */
extern MPT_STRUCT(node) *mpt_node_destroy(MPT_STRUCT(node) *node)
{
	MPT_INTERFACE(metatype) *meta;
	
	if (!node) {
		errno = EFAULT;
		return 0;
	}
	if (node->parent || node->next || node->prev) {
		errno = ENOTSUP;
		return node;
	}
	mpt_node_clear(node);
	
	if ((meta = node->_meta)) {
		meta->_vptr->unref(meta);
	}
	mpt_identifier_set(&node->ident, 0, 0);
	
	free(node);
	
	return 0;
}
