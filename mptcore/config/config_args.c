/*!
 * read basic and specific configuration from files.
 */

#include <stdio.h>
#include <string.h>

#include "node.h"
#include "message.h"
#include "meta.h"

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
	int res, count, val;
	
	if (!args) {
		res = cfg->_vptr->assign(cfg, &p, 0);
		return res < 0 ? res : 0;
	}
	count = 0;
	val = 0;
	p.assign = '=';
	do {
		/* get assign target */
		if ((res = args->_vptr->conv(args, MPT_property_assign('=') | MPT_ENUM(ValueConsume), &pr)) > 0) {
			if (!pr.name) {
				return count ? count : MPT_ERROR(BadArgument);
			}
		}
		/* try to get value data */
		else if ((res = args->_vptr->conv(args, MPT_ENUM(TypeValue) | MPT_ENUM(ValueConsume), &pr.val)) > 0) {
			if (val) {
				return count;
			}
			pr.name = 0;
		}
		/* get simple filename */
		else if ((res = args->_vptr->conv(args, 's' | MPT_ENUM(ValueConsume), &pr.val.ptr)) > 0) {
			pr.name = 0;
			pr.val.fmt = 0;
			pr.val.ptr = 0;
		}
		/* end or error return */
		else {
			return res && !count ? res : count;
		}
		/* only single top level assign */
		if (!pr.name) {
			if (val++) {
				return count;
			}
			p.len = 0;
		} else {
			mpt_path_set(&p, pr.name, -1);
		}
		if (cfg->_vptr->assign(cfg, &p, &pr.val) < 0) {
			return count ? count : MPT_ERROR(BadValue);
		}
		/* assign config */
		++count;
	} while (res & MPT_ENUM(ValueConsume));
	
	return 0;
}

