/*!
 * MPT core library
 *   get config element
 */

#include <string.h>

#include "meta.h"
#include "types.h"

#include "config.h"

/*!
 * \ingroup mptConfig
 * \brief get configuration element
 * 
 * Find element in configuration.
 * 
 * \param dest   element path
 * \param sep    path separator
 * \param assign path end
 * 
 * \return config element if exists
 */
extern MPT_INTERFACE(convertable) *mpt_config_get(const MPT_INTERFACE(config) *conf, const char *dest, int sep, int assign)
{
	MPT_STRUCT(path) p = MPT_PATH_INIT;
	
	if (!conf) {
		MPT_INTERFACE(metatype) *mt;
		
		if (!(mt = mpt_config_global(0))
		    || (MPT_metatype_convert(mt, MPT_ENUM(TypeConfigPtr), &conf) < 0)
		    || !conf) {
			return 0;
		}
	}
	if (dest) {
		const char *end;
		p.sep = sep;
		p.assign = 0;
		mpt_path_set(&p, dest, -1);
		if (assign > 0 && (end = memchr(dest, assign, p.len))) {
			p.assign = assign;
		}
	}
	return conf->_vptr->query(conf, &p);
}
