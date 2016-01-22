
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
	MPT_STRUCT(value) d;
	
	where.sep = sep;
	where.assign = end;
	(void) mpt_path_set(&where, path, -1);
	
	if (!conf) {
		conf = mpt_config_global(0);
	}
	if (!val) {
		return conf->_vptr->assign(conf, &where, 0);
	}
	d.fmt = 0;
	d.ptr = val;
	
	return conf->_vptr->assign(conf, &where, &d);
}
