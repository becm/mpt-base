/*!
 * make shallow copy of node, refere to same storage.
 */

#include <errno.h>

#include "node.h"

extern MPT_STRUCT(node) *mpt_node_clone(const MPT_STRUCT(node) *node, MPT_STRUCT(node) *copy)
{
	MPT_INTERFACE(metatype) *meta, *tmp;
	
	if (!node) {
		errno = EFAULT;
		return 0;
	}
	if (copy == node) {
		return copy;
	}
	/* node has cloneable storage */
	if ((meta = node->_meta) && !(meta = meta->_vptr->addref(meta))) {
		return 0;
	}
	if (copy) {
		if ((tmp = copy->_meta)) {
			tmp->_vptr->unref(tmp);
		}
		copy->_meta = meta;
		return copy;
	}
	if (!(copy = mpt_node_new(0, 0, 0))) {
		if (meta) {
			meta->_vptr->unref(meta);
		}
		return 0;
	}
	if ((tmp = copy->_meta)) {
		tmp->_vptr->unref(tmp);
	}
	copy->_meta = meta;
	
	return copy;
}

