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
 * \param log   logging descriptor
 * 
 * \return string describing error
 */
extern const char *mpt_client_read(MPT_INTERFACE(client) *cl, MPT_INTERFACE(metatype) *args)
{
	MPT_STRUCT(path) p = MPT_PATH_INIT;
	MPT_STRUCT(property) pr;
	int res;
	
	if (!args) {
		if (cl->_vptr->cfg.assign((void *) cl, 0, 0) < 0) {
			return MPT_tr("default client config required");
		}
		return 0;
	}
	/* try to get subtree target for config file */
	if ((res = args->_vptr->conv(args, MPT_property_assign(':') | MPT_ENUM(ValueConsume), &pr)) > 0) {
		if (pr.name) {
			mpt_path_set(&p, pr.name, -1);
		}
	}
	/* try to get value data */
	else if ((res = args->_vptr->conv(args, MPT_ENUM(TypeValue) | MPT_ENUM(ValueConsume), &pr.val)) > 0) {
		pr.name = 0;
	}
	/* get simple filename */
	else if ((res = args->_vptr->conv(args, 's' | MPT_ENUM(ValueConsume), &pr.val.ptr)) >= 0) {
		if (!res || !pr.val.ptr) {
			if (cl->_vptr->cfg.assign((void *) cl, 0, 0) < 0) {
				return MPT_tr("default client config required");
			}
			return 0;
		}
		pr.name = 0;
		pr.val.fmt = 0;
	}
	else {
		return MPT_tr("unable to get file name from argument");
	}
	/* direct assignment from name */
	if (cl->_vptr->cfg.assign((void *) cl, pr.name ? &p : 0, &pr.val) < 0) {
		return MPT_tr("failed to apply configuration");
	}
	return 0;
}

