/*!
 * read basic and specific configuration from files.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

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
extern const char *mpt_client_read(MPT_STRUCT(node) *conf, MPT_INTERFACE(metatype) *args, MPT_INTERFACE(logger) *log)
{
	MPT_STRUCT(node) tmp = MPT_NODE_INIT;
	MPT_STRUCT(property) pr;
	const char *fname;
	FILE *fd;
	int res;
	
	if (!conf) {
		errno = EFAULT;
		return MPT_tr("missing configuration node");
	}
	if (!args) {
		if (!(fname = mpt_node_data(conf, 0))) {
			return MPT_tr("default config file required");
		}
	}
	/* try to get subtree target for config file */
	else if ((res = args->_vptr->conv(args, MPT_property_assign(':') | MPT_ENUM(ValueConsume), &pr)) >= 0) {
		MPT_STRUCT(path) p = MPT_PATH_INIT;
		MPT_STRUCT(node) *sub;
		if (!pr.name) {
			return MPT_tr("unable to get file name from argument");
		}
		if (pr.val.fmt || !(fname = pr.val.ptr)) {
			return MPT_tr("bad file name value type");
		}
		p.sep = '.';
		p.assign = 0;
		if (!(sub = mpt_node_query(conf->children, &p, strlen(fname) + 1))) {
			return MPT_tr("unable to register sub configuration");
		}
		if (!conf->children) {
			conf->children = sub;
		}
		if (p.len && !(conf = mpt_node_query(sub->children, &p, -1))) {
			return MPT_tr("unable to find sub configuration");
		}
	}
	/* save to base node */
	else if ((res = args->_vptr->conv(args, 's' | MPT_ENUM(ValueConsume), &fname)) >= 0) {
		if (!fname) {
			return MPT_tr("unable to get file name from argument");
		}
	}
	else {
		return MPT_tr("bad file name argument type");
	}
	/* atomic read of config file */
	if (!(fd = fopen(fname, "r"))) {
		return MPT_tr("unable to open config file");
	}
	res = mpt_config_read(&tmp, fd, 0, "ns", log);
	fclose(fd);
	
	if (res < 0) {
		return MPT_tr("error while parsing file");
	}
	/* replace node elements */
	mpt_node_clear(conf);
	conf->children = tmp.children;
	
	return 0;
}

