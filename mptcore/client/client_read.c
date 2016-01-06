/*!
 * read basic and specific configuration from files.
 */

#include <stdio.h>
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
	else {
		if ((res = args->_vptr->conv(args, 's' | MPT_ENUM(ValueConsume), &fname)) < 0 || !fname) {
			return MPT_tr("unable to get file name from argument");
		}
	}
	if (!(fd = fopen(fname, "r"))) {
		return MPT_tr("unable to open config file");
	}
	
	res = mpt_config_read(&tmp, fd, 0, "ns", log);
	fclose(fd);
	
	if (res < 0) {
		return MPT_tr("unable to open config file");
	}
	mpt_node_clear(conf);
	conf->children = tmp.children;
	
	return 0;
}

