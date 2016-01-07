
#include <errno.h>

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
extern MPT_STRUCT(node) *mpt_node_assign(MPT_STRUCT(node) **base, const MPT_STRUCT(path) *dest)
{
	MPT_STRUCT(path) path = *dest;
	MPT_STRUCT(node) *conf;
	const char *data;
	
	if (!(conf = mpt_node_query(*base, &path, path.valid + 1))) {
		return 0;
	}
	if (!*base) *base = conf;
	
	if (path.len && !(conf = mpt_node_query(conf->children, &path, -1))) {
		return 0;
	}
	
	if (!path.valid || !(data = mpt_path_data(&path))) {
		data = "";
	}
	mpt_node_set(conf, data);
	
	return conf;
}

