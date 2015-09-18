
#include <errno.h>
#include <string.h>

#include "node.h"
#include "config.h"

#include "client.h"

extern MPT_STRUCT(node) *_mpt_config_root();

/*!
 * \ingroup mptClient
 * \brief get config node for client
 * 
 * Get config node from global configuration.
 * Create and insert if not found.
 * 
 * \param name  identifier of config node
 * 
 * \return configuration node
 */
extern MPT_STRUCT(node) *mpt_client_config(const char *name)
{
	MPT_STRUCT(node) *root, *conf;
	size_t len;
	
	if (!(root = mpt_node_get(0, 0))) {
		errno = EFAULT;
		return 0;
	}
	if ((conf = mpt_node_next(root->children, name))) {
		return conf;
	}
	len = name ? strlen(name) : 0;
	if (!(conf = mpt_node_new(len, 8))) {
		return 0;
	}
	if ((len && !mpt_identifier_set(&conf->ident, name, len))
	    || (mpt_gnode_insert(root, 0, conf) < 0)) {
		mpt_node_destroy(conf);
		return 0;
	}
	return conf;
}

