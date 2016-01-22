
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
extern MPT_INTERFACE(metatype) *mpt_config_get(const MPT_INTERFACE(config) *conf, const char *dest, int sep, int assign)
{
	MPT_STRUCT(path) p = MPT_PATH_INIT;
	
	if (!conf && !(conf = mpt_config_global(0))) {
		return 0;
	}
	if (dest) {
		p.sep = sep;
		p.assign = assign;
		mpt_path_set(&p, dest, -1);
	}
	return conf->_vptr->query(conf, &p);
}
