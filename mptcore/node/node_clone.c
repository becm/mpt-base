/*!
 * make shallow copy of node, refere to same storage.
 */

#include <errno.h>

#include "node.h"

extern MPT_STRUCT(node) *mpt_node_clone(const MPT_STRUCT(node) *node)
{
	MPT_STRUCT(node) *copy;
	MPT_INTERFACE(metatype) *meta, *tmp;
	int nlen;
	
	if (!node) {
		errno = EFAULT;
		return 0;
	}
	/* node has cloneable storage */
	if ((meta = node->_meta) && !(meta = meta->_vptr->clone(meta))) {
		return 0;
	}
	if (!(copy = mpt_node_new(nlen = node->ident._len, 0))) {
		if (meta) {
			meta->_vptr->unref(meta);
		}
		return 0;
	}
	nlen = mpt_identifier_len(&node->ident);
	if (nlen && !mpt_identifier_set(&copy->ident, mpt_identifier_data(&node->ident), nlen)) {
		mpt_node_destroy(copy);
		return 0;
	}
	if ((tmp = copy->_meta)) {
		tmp->_vptr->unref(tmp);
	}
	copy->_meta = meta;
	
	return copy;
}

