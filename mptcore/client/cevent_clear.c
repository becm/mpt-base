/*!
 * execute solver step
 */

#include "event.h"
#include "message.h"
#include "meta.h"

#include "client.h"

/*!
 * \ingroup mptClient
 * \brief config clear
 * 
 * Remove elements of config.
 * 
 * \param cl  config descriptor
 * \param ev  event data
 * 
 * \return hint to event controller (stop/continue/error)
 */
extern int mpt_cevent_clear(MPT_INTERFACE(config) *cl, MPT_STRUCT(event) *ev)
{
	MPT_STRUCT(path) p = MPT_PATH_INIT;
	
	if (!ev) {
		return 0;
	}
	if (!cl) {
		MPT_ABORT("missing client descriptor");
	}
	if (!ev->msg) {
		/* clear single pass data */
		cl->_vptr->remove(cl, &p);
		return MPT_event_stop(ev, MPT_tr("solver cleared"));
	}
	else {
		MPT_INTERFACE(iterator) *src;
		MPT_INTERFACE(metatype) *mt;
		const char *arg;
		int ret;
		
		if (!(src = mpt_event_command(ev))) {
			ev->id = 0;
			return MPT_EVENTFLAG(Fail) | MPT_EVENTFLAG(Default);
		}
		mt = (void *) src;
		if ((ret = mt->_vptr->conv(mt, 's', &arg)) <= 0
		    || (ret = src->_vptr->advance(src)) < 0
		    || (ret = mt->_vptr->conv(mt, 's', &arg)) < 0) {
			src->_vptr->meta.ref.unref((void *) src);
			return MPT_event_fail(ev, MPT_ERROR(BadValue), MPT_tr("bad argument content"));
		}
		/* clear single pass data */
		if ((ret = src->_vptr->advance(src)) < 0) {
			cl->_vptr->remove(cl, &p);
		}
		do {
			if (arg && *arg) {
				mpt_path_set(&p, arg, -1);
				if (cl->_vptr->remove(cl, &p) < 0) {
					mpt_log(0, __func__, MPT_LOG(Info), "%s: %s",
					        MPT_tr("no config element"), arg);
				} else {
					mpt_log(0, __func__, MPT_LOG(Debug2), "%s: %s",
					        MPT_tr("removed config element"), arg);
				}
			}
			if ((ret = mt->_vptr->conv(mt, 's', &arg)) < 0) {
				src->_vptr->meta.ref.unref((void *) src);
				return MPT_event_fail(ev, MPT_ERROR(BadValue), MPT_tr("bad argument content"));
			}
		} while ((ret = src->_vptr->advance(src)) >= 0);
		
		src->_vptr->meta.ref.unref((void *) src);
		return MPT_event_stop(ev, MPT_tr("solver elements cleared"));
	}
}

