/*!
 * configure and prepare bound solver.
 */

#include <string.h>
#include <errno.h>

#include "message.h"
#include "event.h"

#include "client.h"

/*!
 * \ingroup mptClient
 * \brief client preparation
 * 
 * Apply solver configuration from arguments in event data.
 * Prepare client for run and display initial output.
 * 
 * \param cl  client descriptor
 * \param ev  event data
 * 
 * \return hint to event controller (int)
 */
extern int mpt_cevent_prep(MPT_INTERFACE(client) *cl, MPT_STRUCT(event) *ev)
{
	MPT_INTERFACE(metatype) *src;
	int res;
	
	if (!ev) {
		return 0;
	}
	if (!cl) {
		return MPT_ERROR(BadArgument);
	}
	/* apply command line client parameters */
	src = 0;
	if (ev->msg) {
		MPT_STRUCT(message) msg = *ev->msg;
		MPT_STRUCT(msgtype) mt;
		ssize_t part;
		
		if ((part = mpt_message_read(&msg, sizeof(mt), &mt)) < (ssize_t) sizeof(mt)) {
			if (part) return MPT_event_fail(ev, MPT_ERROR(MissingData), MPT_tr("missing message type"));
			return MPT_event_fail(ev, MPT_ERROR(MissingData), MPT_tr("missing message header"));
		}
		if (mt.cmd != MPT_ENUM(MessageCommand)) {
			return MPT_event_fail(ev, MPT_ERROR(BadType), MPT_tr("bad message format"));
		}
		/* consume command part  */
		if ((part = mpt_message_argv(&msg, mt.arg)) >= 0) {
			mpt_message_read(&msg, part+1, 0);
			part = mpt_message_argv(&msg, mt.arg);
		}
		if (part >= 0 && !(src = mpt_meta_message(&msg, mt.arg))) {
			return MPT_event_fail(ev, MPT_ERROR(BadOperation), MPT_tr("failed to create argument stream"));
		}
	}
	/* prepare client */
	res = cl->_vptr->prep(cl, src);
	if (src) src->_vptr->unref(src);
	
	if (res < 0) {
		return MPT_event_fail(ev, res, MPT_tr("client preparation failed"));
	}
	return MPT_event_good(ev, MPT_tr("client preparation completed"));
}
