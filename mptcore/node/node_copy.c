/*!
 * create node with same id/data
 */

#include <string.h>
#include <errno.h>

#include "node.h"

extern MPT_STRUCT(node) *mpt_node_copy(const MPT_STRUCT(node) *node, MPT_STRUCT(node) *copy)
{
	MPT_INTERFACE(metatype) *meta;
	const char *ident = 0, *data = 0;
	size_t dlen = 0;
	int ilen = 0;
	
	if (!node) {
		errno = EFAULT;
		return 0;
	}
	if (copy == node) {
		return copy;
	}
	/* node has storage to be copied */
	if ((meta = node->_meta)) {
		data = meta->_vptr->typecast(meta, 's');
		dlen = data ? strlen(data)+1 : 0;
	}
	if (copy) {
		return mpt_node_set(copy, data) ? copy : 0;
	}
	ident = mpt_identifier_data(&node->ident, 0);
	ilen  = ident ? strlen(ident) : 0;
	
	if (!(copy = mpt_node_new(dlen, ident, ilen))) {
		return 0;
	}
	if (dlen && !mpt_node_set(copy, data)) {
		mpt_node_destroy(copy);
		return 0;
	}
	return copy;
}

