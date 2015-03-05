/*!
 * read basic and specific configuration from files.
 */

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
 * \param conf configuration target
 * \param msg  message data
 * \param sep  message argument separator
 * \param log  logging descriptor
 * 
 * \return string describing error
 */
extern const char *mpt_client_read(MPT_STRUCT(node) *conf, MPT_STRUCT(message) *msg, int sep, MPT_INTERFACE(logger) *log)
{
	const char *fname;
	char fbuf[1024];
	ssize_t part = -1;
	
	if (!conf) {
		errno = EFAULT;
		return MPT_tr("missing configuration node");
	}
	/* use first argument as client configuration */
	if (!msg || (part = mpt_message_argv(msg, sep)) <= 0) {
		if (!(fname = mpt_node_data(conf, 0))) {
			return MPT_tr("no configuration specified");
		}
	} else if ((size_t) part < msg->used && (!sep || !((char *) msg->base)[part])) {
		fname = msg->base;
		part = mpt_message_read(msg, part, 0);
	} else if (part >= (ssize_t) sizeof(fbuf)) {
		return MPT_tr("temporary buffer exceeded");
	} else {
		part = mpt_message_read(msg, part, fbuf);
		fbuf[part] = '\0';
		fname = fbuf;
	}
	/* read client configuration */
	if (mpt_config_read(conf, fname, 0, "ns", log) < 0) {
		return MPT_tr("reading config file failed");
	}
	return 0;
}

