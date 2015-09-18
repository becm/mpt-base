
#include "node.h"
#include "config.h"

/*!
 * \ingroup mptConfig
 * \brief get configuration element
 * 
 * Find element in configuration tree/list.
 * 
 * \param dest   element path
 * \param sep    path separator
 * \param assign path end
 * 
 * \return config element if exists
 */
extern MPT_INTERFACE(metatype) *mpt_config_get(MPT_INTERFACE(config) *conf, const char *dest, int sep, int assign)
{
	MPT_INTERFACE(metatype) **mt;
	MPT_STRUCT(path) p = MPT_PATH_INIT;
	
	p.sep = sep;
	p.assign = assign;
	mpt_path_set(&p, dest, -1);
	
	if (conf) {
		if (!(mt = conf->_vptr->query(conf, &p, -1))) {
			return 0;
		}
	} else {
		MPT_STRUCT(node) *curr;
		if (!(curr = mpt_node_get(0, &p))) {
			return 0;
		}
		mt = &curr->_meta;
	}
	return *mt;
}
