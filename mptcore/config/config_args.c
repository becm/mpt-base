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
		res = cfg->_vptr->assign(cfg, &p, 0);
		return res < 0 ? res : 0;
	}
	count = 0;
	p.assign = '=';
	do {
		/* get assign target */
		if ((res = args->_vptr->conv(args, MPT_ENUM(PropertyEqual) | MPT_ENUM(ValueConsume), &pr)) <= 0) {
			return (res && count) ? count : res;
		}
		/* only single top level assign */
		if (!pr.name) {
			return count ? count : MPT_ERROR(BadType);
		}
		mpt_path_set(&p, pr.name, pr.desc ? (pr.desc - pr.name) : -1);
		if (cfg->_vptr->assign(cfg, &p, &pr.val) < 0) {
			return count ? count : MPT_ERROR(BadValue);
		}
		/* assign config */
		++count;
	} while (res & MPT_ENUM(ValueConsume));
	
	return 0;
}

