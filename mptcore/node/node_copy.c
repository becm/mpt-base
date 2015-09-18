/*!
 * create node with same id/data
 */

#include <string.h>
#include <errno.h>

#include "node.h"

extern MPT_STRUCT(node) *mpt_node_copy(const MPT_STRUCT(node) *node, MPT_STRUCT(node) *copy)
{
	MPT_INTERFACE(metatype) *meta;
	const char *data = 0;
	size_t dlen = 0;
	int nlen;
	
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
	if (!(copy = mpt_node_new(nlen = node->ident._len, dlen))) {
		return 0;
	}
	nlen = mpt_identifier_len(&node->ident);
	if ((nlen && !mpt_identifier_set(&copy->ident, mpt_identifier_data(&node->ident), nlen))
	    || (dlen && !mpt_node_set(copy, data))) {
		mpt_node_destroy(copy);
		return 0;
	}
	return copy;
}

