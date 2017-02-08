/*!
 * execute solver step
 */

#include <stdio.h>
#include <errno.h>

#include "event.h"
#include "message.h"
#include "meta.h"

#include "convert.h"

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
	
	if (!ev) return 0;
	if (!ev->msg) {
		/* clear single pass data */
		cl->_vptr->remove((void *) cl, 0);
		cl->_vptr->remove((void *) cl, &p);
		return MPT_event_stop(ev, MPT_tr("solver cleared"));
	}
	else {
		MPT_INTERFACE(metatype) *src;
		const char *arg;
		int ret;
		
		if (!(src = mpt_event_command(ev))) {
			ev->id = 0;
			return MPT_ENUM(EventFail) | MPT_ENUM(EventDefault);
		}
		if ((ret = src->_vptr->conv(src, 's' | MPT_ENUM(ValueConsume), 0)) <= 0
		    || (ret = src->_vptr->conv(src, 's' | MPT_ENUM(ValueConsume), &arg)) < 0) {
			src->_vptr->ref.unref((void *) src);
			return MPT_event_fail(ev, MPT_ERROR(BadValue), MPT_tr("bad argument content"));
		}
		/* clear single pass data */
		if (!ret) {
			cl->_vptr->remove((void *) cl, 0);
			cl->_vptr->remove((void *) cl, &p);
		}
		do {
			if (arg) {
				mpt_path_set(&p, arg, -1);
				cl->_vptr->remove((void *) cl, &p);
			}
			if ((ret = src->_vptr->conv(src, 's' | MPT_ENUM(ValueConsume), &arg)) < 0) {
				src->_vptr->ref.unref((void *) src);
				return MPT_event_fail(ev, MPT_ERROR(BadValue), MPT_tr("bad argument content"));
			}
		} while (ret & MPT_ENUM(ValueConsume));
		
		src->_vptr->ref.unref((void *) src);
		return MPT_event_good(ev, MPT_tr("solver elements cleared"));
	}
}

