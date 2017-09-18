/*!
 * make shallow copy of node, refere to same storage.
 */

#include <errno.h>

#include "meta.h"

#include "node.h"

extern MPT_STRUCT(node) *mpt_node_clone(const MPT_STRUCT(node) *node)
{
	MPT_STRUCT(node) *copy;
	MPT_INTERFACE(metatype) *meta, *tmp;
	
	if (!node) {
		errno = EFAULT;
		return 0;
	}
	/* node has cloneable storage */
	if ((meta = node->_meta) && !(meta = meta->_vptr->clone(meta))) {
		return 0;
	}
	if (!(copy = mpt_node_new(node->ident._len))) {
		if (meta) {
			meta->_vptr->ref.unref((void *) meta);
		}
		return 0;
	}
	if (!mpt_identifier_copy(&copy->ident, &node->ident)) {
		mpt_node_destroy(copy);
		if (meta) {
			meta->_vptr->ref.unref((void *) meta);
		}
	}
	if ((tmp = copy->_meta)) {
		tmp->_vptr->ref.unref((void *) tmp);
	}
	copy->_meta = meta;
	
	return copy;
}

