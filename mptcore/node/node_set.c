/*!
 * set MPT node metatype data.
 */

#include "meta.h"
#include "object.h"

#include "node.h"

/*!
 * \ingroup mptNode
 * \brief set node data
 * 
 * Set node data to text value.
 * 
 * \param node  node data
 * \param val   value to assign
 * 
 * \return type of created meta element
 */
extern int mpt_node_set(MPT_STRUCT(node) *node, const MPT_STRUCT(value) *val)
{
	
	MPT_INTERFACE(metatype) *old, *mt;
	
	if ((old = node->_meta)) {
		/* try assign existing data */
		MPT_INTERFACE(object) *obj = 0;
		if ((old->_vptr->conv(old, MPT_ENUM(TypeObject), &obj)) >= 0
	            && obj
	            && mpt_object_pset(obj, 0, val, 0) >= 0) {
			return old->_vptr->conv(old, 0, 0);
		}
	}
	/* default config data */
	if (!val) {
		/* try to reset existing iterator */
		MPT_INTERFACE(iterator) *it = 0;
		if (old
		    && old->_vptr->conv(old, MPT_ENUM(TypeIterator), &it) >= 0
		    && it
		    && it->_vptr->reset(it) >= 0) {
			return old->_vptr->conv(old, 0, 0);
		}
		mt = 0;
	}
	/* create new metatype for data */
	else if (!(mt = mpt_meta_new(*val))) {
		return MPT_ERROR(BadOperation);
	}
	if (old) {
		old->_vptr->ref.unref((void *) old);
	}
	node->_meta = mt;
	return mt->_vptr->conv(mt, 0, 0);
}

