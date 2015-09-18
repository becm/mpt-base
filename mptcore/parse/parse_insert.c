
#include <errno.h>

#include "node.h"
#include "config.h"
#include "parse.h"

/*!
 * \ingroup mptParse
 * \brief insert path element
 * 
 * insert parsed element into configuration.
 * 
 * \param old   configuration list root
 * \param dest  element destination and data
 * 
 * \return (new/changed) configuration list
 */
extern MPT_STRUCT(node) *mpt_parse_insert(MPT_STRUCT(node) *old, const MPT_STRUCT(path) *dest)
{
	MPT_STRUCT(path) path = *dest;
	MPT_STRUCT(node) *conf;
	const char *data;
	
	if (!(conf = mpt_node_query(old, &path, path.valid + 1))) {
		return 0;
	}
	if (!old) old = conf;
	
	if (path.len && !(conf = mpt_node_get(conf->children, &path))) {
		return 0;
	}
	
	if (!path.valid || !(data = mpt_path_data(&path))) {
		data = "";
	}
	mpt_node_set(conf, data);
	
	return old;
}

