/*!
 * set MPT node metatype data.
 */

#include <errno.h>
#include <string.h>

#include "node.h"

/*!
 * \ingroup mptNode
 * \brief set node data
 * 
 * Set node data to text value.
 * 
 * \param node  node data
 * \param data  string to assign
 * 
 * \return number of moved elemets
 */
extern int mpt_node_set(MPT_STRUCT(node) *node, const char *data)
{
	MPT_INTERFACE(metatype) *old, *replace;
	int ret = 0;
	
	if (((old = node->_meta)
	     && (ret = mpt_meta_set(old, 0, "s", data)) >= 0)
	    || !data) {
		return ret;
	}
	if (!(replace = mpt_meta_new(strlen(data)))) {
		return -1;
	}
	if ((ret = mpt_meta_set(replace, 0, "s", data)) < 0) {
		replace->_vptr->unref(replace);
		return -3;
	}
	old->_vptr->unref(old);
	
	node->_meta = replace;
	
	return ret;
}

