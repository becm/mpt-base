/*!
 * client initialisation event.
 */

#include <string.h>

#include "node.h"
#include "array.h"
#include "config.h"
#include "message.h"
#include "event.h"

#include "client.h"

static int setConfig(void *udata, MPT_STRUCT(property) *pr)
{
	MPT_STRUCT(node) *conf, *base = *((MPT_STRUCT(node) **) udata);
	MPT_STRUCT(path) where = MPT_PATH_INIT;
	size_t len;
	
	where.sep = '.';
	where.assign = 0;
	
	if (pr->val.fmt) {
		return -1;
	}
	
	len = mpt_path_set(&where, pr->name, -1);
	len = strlen(pr->val.ptr);
	/* max length exceeded */
	if (++len > INT32_MAX) return -2;
	
	conf = base ? base->children : 0;
	
	if (!(conf = mpt_node_query(conf, &where, len))) {
		return -2;
	}
	if (!base) {
		if (!(base = mpt_client_config("client"))) {
			mpt_node_destroy(conf);
			return -3;
		}
		*((MPT_STRUCT(node) **) udata) = base;
		base->children = conf;
	}
	else if (!base->children) {
		base->children = conf;
	}
	if (where.len && !(conf = mpt_node_get(conf->children, &where))) {
		MPT_ABORT("failed to get created node");
	}
	if (mpt_node_set(conf, pr->val.ptr) < 0) {
		return -1;
	}
	return 0;
}

/*!
 * \ingroup mptClient
 * \brief call client initialisation
 * 
 * Apply client configuration from arguments in event data.
 * Initialize client data (solver binding).
 * 
 * \param cl  client descriptor
 * \param ev  event data
 * 
 * \return hint to event controller (int)
 */
extern int mpt_cevent_init(MPT_INTERFACE(client) *cl, MPT_STRUCT(event) *ev)
{
	MPT_STRUCT(msgtype) mt = MPT_MSGTYPE_INIT;
	
	if (!ev) {
		return 0;
	}
	if (ev->msg) {
		MPT_STRUCT(message) msg = *ev->msg;
		ssize_t part = 0;
		
		if (mpt_message_read(&msg, sizeof(mt), &mt) < (ssize_t) sizeof(mt)
		    || mt.cmd != MPT_ENUM(MessageCommand)) {
			mpt_output_log(cl->out, __func__, MPT_ENUM(LogError) | MPT_ENUM(LogFunction), "%s",
			               MPT_tr("bad message format"));
			return -1;
		}
		else if ((part = mpt_message_argv(&msg, mt.arg)) > 0) {
			mpt_message_read(&msg, part, 0);
		}
		/* set solver parameters from arguments */
		while (part > 0) {
			if (!(part = mpt_message_pset(&msg, mt.arg, setConfig, &cl->conf))) {
				mpt_output_log(cl->out, __func__, MPT_ENUM(LogError) | MPT_ENUM(LogFunction), "%s",
				               MPT_tr("error while setting client configuration"));
				part = 1;
			}
		}
	}
	if (!cl) {
		MPT_ABORT("missing client descriptor");
	}
	mpt_output_log(cl->out, __func__, 0, 0);
	
	/* initialize and bind solver */
	if (cl->_vptr->init(cl) < 0) {
		return MPT_event_fail(ev, MPT_tr("unable to initialize client data"));
	}
	return MPT_event_good(ev, MPT_tr("client configuration successfull"));
}

