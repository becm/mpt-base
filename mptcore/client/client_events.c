/*!
 * setup event controller for solver events
 */

#include <stdio.h>
#include <inttypes.h>

#include <string.h>
#include <sys/uio.h>

#include "array.h"

#include "message.h"
#include "event.h"

#include "meta.h"
#include "output.h"
#include "convert.h"

#include "client.h"

static int clientConfig(MPT_INTERFACE(client) *cl, MPT_STRUCT(event) *ev)
{
	static const char defFmt[] = "no default configuration state";
	
	int err;
	
	if (!ev) {
		return 0;
	}
	if (!ev->msg) {
		if ((err = mpt_config_args((void *) cl, 0)) < 0) {
			return MPT_event_fail(ev, err, MPT_tr(defFmt));
		}
	}
	else {
		MPT_INTERFACE(metatype) *args = 0;
		const char *cmd;
		
		if (!(args = mpt_event_command(ev))) {
			ev->id = 0;
			return MPT_ENUM(EventFail) | MPT_ENUM(EventDefault);
		}
		/* consume command */
		args->_vptr->conv(args, 's' | MPT_ENUM(ValueConsume), &cmd);
		/* process config arguments */
		err = mpt_config_args((void *) cl, args);
		args->_vptr->ref.unref((void *) args);
		
		if (err < 0) {
			return MPT_event_fail(ev, err, MPT_tr("bad client config element"));
		}
		if (err) {
			char buf[128];
			snprintf(buf, sizeof(buf), "%s: %d", MPT_tr("compatible argument count"), err);
			return MPT_event_good(ev, buf);
		}
	}
	return MPT_event_good(ev, MPT_tr("arguments applied to client configuration"));
}

static int clientClose(MPT_INTERFACE(client) *cl, MPT_STRUCT(event) *ev)
{
	MPT_STRUCT(msgtype) mt = MPT_MSGTYPE_INIT;
	if (!ev) {
		cl->_vptr->cfg.ref.unref((void *) cl);
		return 0;
	}
	if (ev->msg) {
		MPT_STRUCT(message) msg = *ev->msg;
		ssize_t part;
		
		if ((part = mpt_message_read(&msg, sizeof(mt), &mt)) < (ssize_t) sizeof(mt)) {
			if (part) return MPT_event_fail(ev, MPT_ERROR(MissingData), MPT_tr("missing message type"));
			return MPT_event_fail(ev, MPT_ERROR(MissingData), MPT_tr("missing message header"));
		}
	}
	return MPT_event_term(ev, MPT_tr("terminate event loop"));
}
static const struct
{
	const char *name;
	int (*ctl)(MPT_INTERFACE(client) *, MPT_STRUCT(event) *);
}
cmdsolv[] = {
	{"set",     clientConfig    },
	{"close",   clientClose     },
	{"init",    mpt_cevent_init },
	{"step",    mpt_cevent_step }
};

static int clientCont(void *ptr, MPT_STRUCT(event) *ev)
{
	MPT_STRUCT(dispatch) *d = ptr;
	const char *cmd = "step";
	uintptr_t id;
	
	if (!ev) {
		return 0;
	}
	if (ev->msg) {
		MPT_INTERFACE(metatype) *args;
		const char *next;
		int ret;
		
		if (!(args = mpt_event_command(ev))) {
			ev->id = 0;
			return MPT_ENUM(EventFail) | MPT_ENUM(EventDefault);
		}
		/* use second command element */
		if ((ret = args->_vptr->conv(args, 's' | MPT_ENUM(ValueConsume), &next)) > 0
		    && (ret & MPT_ENUM(ValueConsume))
		    && (ret = args->_vptr->conv(args, 's' | MPT_ENUM(ValueConsume), &next)) > 0
		    && next) {
			cmd = next;
		}
		args->_vptr->ref.unref((void *) args);
	}
	id = mpt_hash(cmd, strlen(cmd));
	
	if (!mpt_command_get(&d->_d, id)) {
		char buf[128];
		snprintf(buf, sizeof(buf), "%s (%"PRIxPTR")", MPT_tr("invalid default command id"), id);
		return MPT_event_fail(ev, MPT_ERROR(BadValue), buf);
	}
	mpt_event_reply(ev, 2, "%s: %s (%"PRIxPTR")", MPT_tr("register default event"), cmd, id);
	
	ev->id = id;
	
	return MPT_ENUM(EventDefault);
}

static int clientStop(void *ptr, MPT_STRUCT(event) *ev)
{
	MPT_STRUCT(msgtype) mt = MPT_MSGTYPE_INIT;
	MPT_STRUCT(dispatch) *d = ptr;
	
	if (!ev) return 0;
	if (ev->msg) {
		MPT_STRUCT(message) msg = *ev->msg;
		ssize_t part;
		
		if ((part = mpt_message_read(&msg, sizeof(mt), &mt)) < (ssize_t) sizeof(mt)) {
			if (part) return MPT_event_fail(ev, MPT_ERROR(MissingData), MPT_tr("missing message type"));
			return MPT_event_fail(ev, MPT_ERROR(MissingData), MPT_tr("missing message type"));
		}
	}
	if (d->_def) {
		mpt_event_reply(ev, 2, "%s (%"PRIxPTR")", MPT_tr("clear default event"), d->_def);
	} else {
		mpt_event_reply(ev, 0, "%s", MPT_tr("clear default event"));
	}
	ev->id = 0;
	
	return MPT_ENUM(EventDefault);
}

/*!
 * \ingroup mptClient
 * \brief register client events
 * 
 * Set event handlers in dispatch descriptor.
 * 
 * \param dsp  dispatch descriptor
 * \param cl   client descriptor
 * 
 * \retval 0  success
 * \retval -1 missing descriptor pointer
 */
extern int mpt_client_events(MPT_STRUCT(dispatch) *dsp, MPT_INTERFACE(client) *cl)
{
	uintptr_t id;
	size_t i;
	
	if (!dsp || !cl) {
		return MPT_ERROR(BadArgument);
	}
	/* mapping of command type messages */
	id = MPT_ENUM(MessageCommand);
	if (mpt_dispatch_set(dsp, id, (int (*)()) mpt_dispatch_hash, dsp) < 0) {
		mpt_log(0, __func__, MPT_LOG(Error), "%s",
		        MPT_tr("unable to set string command handler"));
		return -1;
	}
	/* register default event activator */
	id = mpt_hash("cont", 4);
	if (mpt_dispatch_set(dsp, id, clientCont, dsp) < 0) {
		mpt_log(0, __func__, MPT_LOG(Error), "%s: %"PRIxPTR" (%s)\n",
		        MPT_tr("error registering handler id"), id, "cont");
	}
	/* register default event activator */
	id = mpt_hash("stop", 4);
	if (mpt_dispatch_set(dsp, id, clientStop, dsp) < 0) {
		mpt_log(0, __func__, MPT_LOG(Error), "%s: %"PRIxPTR" (%s)\n",
		        MPT_tr("error registering handler id"), id, "stop");
	}
	/* register client command handler */
	for (i = 0; i < MPT_arrsize(cmdsolv); i++) {
		id = mpt_hash(cmdsolv[i].name, strlen(cmdsolv[i].name));
		
		if (mpt_dispatch_set(dsp, id, (int (*)()) cmdsolv[i].ctl, cl) < 0) {
			mpt_log(0, __func__, MPT_LOG(Warning), "%s: %"PRIxPTR" (%s)\n",
			        MPT_tr("error registering handler id"), id, cmdsolv[i].name);
		}
	}
	return 0;
}
