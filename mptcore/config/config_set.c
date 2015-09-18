
#include <string.h>
#include <errno.h>

#include "node.h"
#include "config.h"

/*!
 * \ingroup mptConfig
 * \brief set configuration
 * 
 * Set element in configuration tree.
 * Use zero-pointer for value to delete element.
 * 
 * \param conf  configuration interface
 * \param path  element position
 * \param val   element data
 * \param sep   path separator
 * \param end   path end delimiter
 * 
 * \return configuration element
 */
extern int mpt_config_set(MPT_INTERFACE(config) *conf, const char *path, const char *val, int sep, int end)
{
	MPT_STRUCT(path) where = MPT_PATH_INIT;
	MPT_INTERFACE(metatype) **mt;
	MPT_STRUCT(node) *n;
	int len;
	
	where.sep = sep;
	where.assign = end;
	len = mpt_path_set(&where, path, -1);
	
	if (!val) {
		MPT_STRUCT(node) *root;
		where.valid = 0;
		if (conf) {
			conf->_vptr->remove(conf, &where);
			return 0;
		}
		root = mpt_node_get(0, 0);
		if (!(n = mpt_node_get(root, &where))) {
			return 0;
		}
		if (n != root && !n->children) {
			mpt_node_unlink(n);
			mpt_node_destroy(n);
			return 3;
		}
		else if (n->_meta) {
			n->_meta->_vptr->unref(n->_meta);
			n->_meta = 0;
			return 2;
		}
		return 1;
	}
	len = strlen(val) + 1;
	
	if (!conf) {
		if (!(n = mpt_node_query(mpt_node_get(0, 0), &where, len))) {
			return -1;
		}
		if (where.len) n = mpt_node_get(n->children, &where);
		
		if (!n || !n->_meta) {
			errno = EINVAL;
			return -3;
		}
		mt = &n->_meta;
	}
	else if (!(mt = conf->_vptr->query(conf, &where, len)) || !mt) {
		return -1;
	}
	if (mpt_meta_set(*mt, 0, "s", val) < 0) {
		errno = ENOTSUP;
		return -1;
	}
	return 0;
}
