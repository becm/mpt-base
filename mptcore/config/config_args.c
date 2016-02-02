/*!
 * read basic and specific configuration from files.
 */

#include <stdio.h>
#include <string.h>

#include "node.h"
#include "message.h"
#include "config.h"

#include "client.h"

/*!
 * \ingroup mptClient
 * \brief read client configuration
 * 
 * Try to get client and solver configuration files from message,
 * fall back to values in client configuration and read files.
 * 
 * \param conf  configuration target
 * \param args  source for files to read
 * 
 * \return string describing error
 */
extern int mpt_config_args(MPT_INTERFACE(config) *cfg, MPT_INTERFACE(metatype) *args)
{
	MPT_STRUCT(path) p = MPT_PATH_INIT;
	MPT_STRUCT(property) pr;
	int res, count;
	
	if (!args) {
		if ((res = cfg->_vptr->assign(cfg, 0, 0)) < 0) {
			return res;
		}
		return 0;
	}
	count = 0;
	do {
		/* try to get subtree target for config file */
		if ((res = args->_vptr->conv(args, MPT_property_assign(':') | MPT_ENUM(ValueConsume), &pr)) > 0) {
			if (pr.name) {
				mpt_path_set(&p, pr.name, -1);
			}
			else if (count) {
				return count;
			}
		}
		/* try to get value data */
		else if ((res = args->_vptr->conv(args, MPT_ENUM(TypeValue) | MPT_ENUM(ValueConsume), &pr.val)) > 0) {
			if (count) {
				return count;
			}
			pr.name = 0;
		}
		/* get simple filename */
		else if ((res = args->_vptr->conv(args, 's' | MPT_ENUM(ValueConsume), &pr.val.ptr)) > 0) {
			if (count) {
				return count;
			}
			pr.name = 0;
			pr.val.fmt = 0;
		}
		else if (!res) {
			if (!count) {
				res = cfg->_vptr->assign(cfg, 0, 0);
			}
			return res;
		}
		else {
			return count ? count : res;
		}
		/* assign config */
		if ((res = cfg->_vptr->assign(cfg, pr.name ? &p : 0, &pr.val)) < 0) {
			return count ? count : res;
		}
		++count;
	} while (res);
	
	return 0;
}

