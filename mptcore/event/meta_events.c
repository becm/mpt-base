/*!
 * setup event controller for solver events
 */

#include <inttypes.h>
#include <string.h>

#include "message.h"
#include "config.h"
#include "event.h"

#include "meta.h"

#include "client.h"

static int setEvent(void *ptr, MPT_STRUCT(event) *ev)
{
	static const char defFmt[] = "no default configuration state";
	
	MPT_INTERFACE(metatype) *mt = ptr;
	MPT_INTERFACE(logger) *log = 0;
	MPT_INTERFACE(config) *cfg = 0;
	int err;
	
	if (!ev) {
		return 0;
	}
	if ((err = mt->_vptr->conv(mt, MPT_ENUM(TypeConfig), &cfg)) < 0) {
		return MPT_event_fail(ev, err, MPT_tr("no config element"));
	}
	mt->_vptr->conv(mt, MPT_ENUM(TypeLogger), &log);
	
	if (!ev->msg) {
		if ((err = mpt_config_args(cfg, 0, log)) < 0) {
			return MPT_event_fail(ev, err, MPT_tr(defFmt));
		}
	}
	else {
		MPT_INTERFACE(iterator) *it;
		
		if (!(mt = mpt_event_command(ev))
		    || mt->_vptr->conv(mt, MPT_ENUM(TypeIterator), &it) < 0
		    || !it) {
			ev->id = 0;
			return MPT_EVENTFLAG(Fail) | MPT_EVENTFLAG(Default);
		}
		/* process config arguments */
		err = mpt_config_args(cfg, it, log);
		mt->_vptr->ref.unref((void *) mt);
		
		if (err < 0) {
			return MPT_event_fail(ev, err, MPT_tr("bad config element"));
		}
		if (err) {
			mpt_context_reply(ev->reply, 0, "%s: %d", MPT_tr("compatible argument count"), err);
			return MPT_EVENTFLAG(None);
		}
	}
	return MPT_event_good(ev, MPT_tr("arguments applied to configuration"));
}
static int delEvent(void *ptr, MPT_STRUCT(event) *ev)
{
	static const char defFmt[] = "no default configuration state";
	
	MPT_INTERFACE(metatype) *mt = ptr;
	MPT_INTERFACE(logger) *log = 0;
	MPT_INTERFACE(config) *cfg = 0;
	int err;
	
	if (!ev) {
		return 0;
	}
	if ((err = mt->_vptr->conv(mt, MPT_ENUM(TypeConfig), &cfg)) < 0
	    || !cfg) {
		return MPT_event_fail(ev, err, MPT_tr("no config element"));
	}
	mt->_vptr->conv(mt, MPT_ENUM(TypeLogger), &log);
	
	if (!ev->msg) {
		if ((err = mpt_config_clear(cfg, 0, log)) < 0) {
			return MPT_event_fail(ev, err, MPT_tr(defFmt));
		}
	}
	else {
		MPT_INTERFACE(iterator) *it;
		
		if (!(mt = mpt_event_command(ev))
		    || mt->_vptr->conv(mt, MPT_ENUM(TypeIterator), &it) < 0
		    || !it) {
			ev->id = 0;
			return MPT_EVENTFLAG(Fail) | MPT_EVENTFLAG(Default);
		}
		/* process config arguments */
		err = mpt_config_clear(cfg, it, log);
		mt->_vptr->ref.unref((void *) mt);
		
		if (err < 0) {
			return MPT_event_fail(ev, err, MPT_tr("bad config element"));
		}
		if (err) {
			mpt_context_reply(ev->reply, 0, "%s: %d", MPT_tr("compatible argument count"), err);
			return MPT_EVENTFLAG(None);
		}
	}
	return MPT_event_good(ev, MPT_tr("arguments removed from configuration"));
}

static int closeEvent(void *ptr, MPT_STRUCT(event) *ev)
{
	MPT_INTERFACE(reference) *ref = ptr;
	if (!ev) {
		ref->_vptr->unref(ref);
		return 0;
	}
	return MPT_event_term(ev, MPT_tr("terminate event loop"));
}

static int stopEvent(void *ptr, MPT_STRUCT(event) *ev)
{
	MPT_STRUCT(dispatch) *d = ptr;
	
	if (!ev) {
		return 0;
	}
	if (d->_def) {
		mpt_context_reply(ev->reply, 0, "%s (%" PRIxPTR ")", MPT_tr("clear default event"), d->_def);
	} else {
		mpt_context_reply(ev->reply, 2, "%s", MPT_tr("no default event"));
	}
	ev->id = 0;
	
	return MPT_EVENTFLAG(Default);
}

static int hashEvent(void *ptr, MPT_STRUCT(event) *ev)
{
	if (!ev) {
		return 0;
	}
	return mpt_dispatch_hash(ptr, ev);
}
static int clientEvent(void *ptr, MPT_STRUCT(event) *ev)
{
	MPT_INTERFACE(client) *cl = ptr;
	if (!ev) {
		cl->_vptr->meta.ref.unref((void *) cl);
		return 0;
	}
	return cl->_vptr->dispatch(cl, ev);
}

/*!
 * \ingroup mptClient
 * \brief register client events
 * 
 * Set event handlers in dispatch descriptor.
 * 
 * \param dsp  dispatch descriptor
 * \param meta target metatype
 * 
 * \retval 0  success
 * \retval -1 missing descriptor pointer
 */
extern int mpt_meta_events(MPT_STRUCT(dispatch) *dsp, MPT_INTERFACE(metatype) *mt)
{
	MPT_INTERFACE(client) *cl;
	uintptr_t id;
	
	if (!dsp || !mt) {
		return MPT_ERROR(BadArgument);
	}
	/* enable client dispatch */
	id = MPT_MESGTYPE(Command);
	cl = 0;
	if (mt->_vptr->conv(mt, mpt_client_typeid(), &cl) >= 0
	    && cl) {
		if (mpt_dispatch_set(dsp, id, clientEvent, cl) < 0) {
			mpt_log(0, __func__, MPT_LOG(Error), "%s: %" PRIxPTR " (%s)\n",
			        MPT_tr("error registering handler id"), id, "stop");
			
		}
		return 0;
	}
	/* mapping of command type messages */
	/* terminate loop */
	id = mpt_hash("close", 5);
	if (mpt_dispatch_set(dsp, id, closeEvent, mt) < 0) {
		mpt_log(0, __func__, MPT_LOG(Error), "%s: %" PRIxPTR " (%s)\n",
		        MPT_tr("error registering handler id"), id, "close");
		return MPT_ERROR(BadOperation);
	}
	if (mpt_dispatch_set(dsp, id, hashEvent, dsp) < 0) {
		mpt_log(0, __func__, MPT_LOG(Error), "%s",
		        MPT_tr("unable to set string command handler"));
	}
	/* disable default event */
	id = mpt_hash("stop", 4);
	if (mpt_dispatch_set(dsp, id, stopEvent, dsp) < 0) {
		mpt_log(0, __func__, MPT_LOG(Error), "%s: %" PRIxPTR " (%s)\n",
		        MPT_tr("error registering handler id"), id, "stop");
	}
	if (mt->_vptr->conv(mt, MPT_ENUM(TypeConfig), 0) < 0) {
		return 0;
	}
	/* enable configuration dispatch */
	id = mpt_hash("set", 3);
	if (mpt_dispatch_set(dsp, id, setEvent, mt) < 0) {
		mpt_log(0, __func__, MPT_LOG(Error), "%s: %" PRIxPTR " (%s)\n",
		        MPT_tr("error registering handler id"), id, "stop");
	}
	id = mpt_hash("del", 3);
	if (mpt_dispatch_set(dsp, id, delEvent, mt) < 0) {
		mpt_log(0, __func__, MPT_LOG(Error), "%s: %" PRIxPTR " (%s)\n",
		        MPT_tr("error registering handler id"), id, "del");
	}
	return 0;
}
