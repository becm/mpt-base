/*!
 * set MPT node metatype data.
 */

#include <errno.h>
#include <string.h>

#include "meta.h"

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
	MPT_STRUCT(value) val;
	int ret = 0;
	
	val.fmt = 0;
	val.ptr = data;
	
	if (((old = node->_meta)
	     && (ret = old->_vptr->assign(old, &val)) >= 0)
	    || !data) {
		return ret;
	}
	if (!(replace = mpt_meta_new(strlen(data)))) {
		return -1;
	}
	if ((ret = replace->_vptr->assign(old, &val)) < 0) {
		replace->_vptr->ref.unref((void *) replace);
		return -3;
	}
	if (old) {
		old->_vptr->ref.unref((void *) old);
	}
	node->_meta = replace;
	
	return ret;
}

