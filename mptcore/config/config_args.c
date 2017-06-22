/*!
 * read basic and specific configuration from files.
 */

#include "client.h"

#include <stdio.h>
#include <string.h>

#include "node.h"
#include "message.h"
#include "meta.h"

#include "config.h"

/*!
 * \ingroup mptClient
 * \brief read client configuration
 * 
 * Try to get client and solver configuration files from message,
 * fall back to values in client configuration and read files.
 * 
 * \param cfg   configuration target
 * \param args  source for files to read
 * 
 * \return string describing error
 */
extern int mpt_config_args(MPT_INTERFACE(config) *cfg, MPT_INTERFACE(iterator) *args)
{
	MPT_STRUCT(path) p = MPT_PATH_INIT;
	MPT_STRUCT(property) pr;
	int res, count;
	
	if (!args) {
		res = cfg->_vptr->assign(cfg, &p, 0);
		return res < 0 ? res : 0;
	}
	count = 0;
	p.assign = '=';
	while (1) {
		/* get assign target */
		if ((res = args->_vptr->get(args, MPT_ENUM(TypeProperty), &pr)) < 0) {
			const char *end;
			res = args->_vptr->get(args, 's', &pr.name);
			if (res < 0) {
				return (res && count) ? count : res;
			}
			if (!(end = pr.name) || !(end = strchr(end, '='))) {
				return count ? count : MPT_ERROR(BadValue);
			}
			mpt_path_set(&p, pr.name, end - pr.name);
			pr.val.fmt = 0;
			pr.val.ptr = end;
		}
		/* only single top level assign */
		else if (!pr.name) {
			return count ? count : MPT_ERROR(BadType);
		}
		else {
			mpt_path_set(&p, pr.name, -1);
		}
		if (cfg->_vptr->assign(cfg, &p, &pr.val) < 0) {
			return count ? count : MPT_ERROR(BadValue);
		}
		/* assign config */
		++count;
	} while ((res = args->_vptr->advance(args)) >= 0);
	
	return 0;
}

