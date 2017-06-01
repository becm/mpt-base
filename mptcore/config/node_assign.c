
#include <sys/uio.h>

#include "node.h"

#include "config.h"

/*!
 * \ingroup mptConfig
 * \brief insert path element
 * 
 * insert path element into tree.
 * 
 * \param base  configuration list reference
 * \param dest  element destination and data
 * 
 * \return (new/changed) configuration list
 */
extern MPT_STRUCT(node) *mpt_node_assign(MPT_STRUCT(node) **base, const MPT_STRUCT(path) *dest, const MPT_STRUCT(value) *val)
{
	static const MPT_STRUCT(value) def = MPT_VALUE_INIT;
	MPT_STRUCT(path) path = *dest;
	MPT_STRUCT(node) *conf;
	
	if (!(conf = mpt_node_query(*base, &path, val ? val : &def))) {
		return 0;
	}
	if (!*base) *base = conf;
	
	if (path.len && !(conf = mpt_node_query(conf->children, &path, 0))) {
		return 0;
	}
	return conf;
}

