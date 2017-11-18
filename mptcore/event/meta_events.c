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

/* config events */
static int setEvent(void *ptr, MPT_STRUCT(event) *ev)
{
	static const char defFmt[] = "no default configuration state";
	
	MPT_INTERFACE(metatype) *mt = ptr;
	MPT_INTERFACE(config) *cfg = 0;
	MPT_INTERFACE(iterator) *it;
	int err;
	
	if (!ev) {
		return 0;
	}
	if ((err = mt->_vptr->conv(mt, MPT_ENUM(TypeConfig), &cfg)) < 0
	    || !cfg) {
		return MPT_event_fail(ev, err, MPT_tr("no config element"));
	}
	
	if (!ev->msg) {
		if ((err = mpt_config_args(cfg, 0)) < 0) {
			return MPT_event_fail(ev, err, MPT_tr(defFmt));
		}
		return MPT_event_good(ev, MPT_tr("assigned default config state"));
	}
	if (!(mt = mpt_event_command(ev))
	    || mt->_vptr->conv(mt, MPT_ENUM(TypeIterator), &it) < 0
	    || !it) {
		ev->id = 0;
		return MPT_EVENTFLAG(Fail) | MPT_EVENTFLAG(Default);
	}
	/* process config arguments */
	err = mpt_config_args(cfg, it);
	mt->_vptr->ref.unref((void *) mt);
	
	if (err < 0) {
		return MPT_event_fail(ev, err, MPT_tr("bad config element"));
	}
	if (err) {
		static const char _fcn[] = "mpt::config::dispatch";
		MPT_INTERFACE(logger) *log = 0;
		const char *val = 0;
		it->_vptr->get(it, 's', &val);
		mt->_vptr->conv(mt, MPT_ENUM(TypeLogger), &log);
		if (val) {
			mpt_log(log, _fcn, MPT_LOG(Warning), "%s: %d", MPT_tr("incompatible argument"), err);
		} else {
			mpt_log(log, _fcn, MPT_LOG(Warning), "%s: %d", MPT_tr("incompatible argument"), err);
		}
		mpt_context_reply(ev->reply, err, "%s", MPT_tr("incomplete assignment"));
		ev->id = 0;
		return MPT_EVENTFLAG(Default);
	}
	return MPT_event_good(ev, MPT_tr("arguments applied to configuration"));
}
static int delEvent(void *ptr, MPT_STRUCT(event) *ev)
{
	static const char defFmt[] = "no default configuration state";
	
	MPT_INTERFACE(metatype) *mt = ptr;
		MPT_INTERFACE(iterator) *it;
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
		return MPT_event_good(ev, MPT_tr("configuration cleared"));
	}
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
	return MPT_event_good(ev, MPT_tr("arguments removed from configuration"));
}
/* dispatcher events */
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
/* termination event */
static int closeEvent(void *ptr, MPT_STRUCT(event) *ev)
{
	MPT_INTERFACE(reference) *ref = ptr;
	if (!ev) {
		ref->_vptr->unref(ref);
		return 0;
	}
	return MPT_event_term(ev, MPT_tr("terminate event loop"));
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
 * \retval <0 failed to register reference
 */
extern int mpt_meta_events(MPT_STRUCT(dispatch) *dsp, MPT_INTERFACE(metatype) *mt)
{
	uintptr_t id;
	
	if (!dsp || !mt) {
		return MPT_ERROR(BadArgument);
	}
	/* terminate loop */
	id = mpt_hash("close", 5);
	if (mpt_dispatch_set(dsp, id, closeEvent, mt) < 0) {
		mpt_log(0, __func__, MPT_LOG(Error), "%s: %" PRIxPTR " (%s)\n",
		        MPT_tr("error registering handler id"), id, "close");
		return MPT_ERROR(BadOperation);
	}
	if (mpt_dispatch_set(dsp, MPT_MESGTYPE(Command), hashEvent, dsp) < 0) {
		MPT_STRUCT(command) *cmd;
		cmd = mpt_command_get(&dsp->_d, id);
		if (!cmd) {
			MPT_ABORT("lost command with registered reference");
		}
		mpt_log(0, __func__, MPT_LOG(Error), "%s",
		        MPT_tr("unable to set string command handler"));
		/* invalidate 'close' command without metatype deref */
		cmd->cmd = 0;
		cmd->arg = 0;
		return MPT_ERROR(BadOperation);
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
