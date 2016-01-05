
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
	MPT_INTERFACE(metatype) **mp;
	MPT_INTERFACE(metatype) *mt;
	MPT_STRUCT(value) d;
	
	where.sep = sep;
	where.assign = end;
	(void) mpt_path_set(&where, path, -1);
	
	if (!conf) {
		conf = mpt_config_global(0);
	}
	if (!val) {
		if (!(mp = conf->_vptr->query(conf, &where, 0))) {
			return MPT_ERROR(BadValue);
		}
		return 0;
	}
	d.fmt = 0;
	d.ptr = val;
	
	if (!(mp = conf->_vptr->query(conf, &where, &d))) {
		return MPT_ERROR(BadType);
	}
	if ((mt = *mp)) {
		return 0;
	}
	if (!(mt = mpt_meta_new(strlen(val)+1)) || mt->_vptr->assign(mt, &d) < 0) {
		return MPT_ERROR(BadValue);
	}
	*mp = mt;
	
	return 1;
}
