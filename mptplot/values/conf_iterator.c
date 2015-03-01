
#include "array.h"
#include "node.h"

#include "values.h"

/*!
 * \ingroup mptValues
 * \brief set client data iterator
 * 
 * Create iterator from config node.
 * 
 * \param it   iterator descriptor pointer
 * \param conf iterator configuration node
 * 
 * \return state of set operation
 */
extern int mpt_conf_iterator(MPT_INTERFACE(iterator) **it, const MPT_STRUCT(node) *conf)
{
	MPT_INTERFACE(iterator) *old, *iter;
	const char *data;
	
	/* keep iterator on default value */
	if (!conf) return *it ? 0 : -1;
	
	/* default intervals for time steps */
	data = conf ? mpt_node_data(conf, 0) : 0;
	iter = mpt_iterator_create(data ? data : "lin 0 1");
	
	if (!iter) return *it ? 0 : -1;
	
	if ((old = (void *) (*it))) {
		old->_vptr->unref(old);
	}
	*it = iter;
	
	return 1;
}

