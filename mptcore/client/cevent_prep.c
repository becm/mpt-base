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
		errno = EFAULT;
		return-1;
	}
	/* apply command line client parameters */
	src = 0;
	if (ev->msg) {
		MPT_STRUCT(message) msg = *ev->msg;
		MPT_STRUCT(msgtype) mt;
		ssize_t part = 0;
		if ((part = mpt_message_read(&msg, sizeof(mt), &mt)) < (ssize_t) sizeof(mt)
		    || mt.cmd != MPT_ENUM(MessageCommand)) {
			mpt_output_log(cl->out, __func__, MPT_FCNLOG(Error), "%s",
			               MPT_tr("bad message format"));
			return -1;
		}
		if (!(src = mpt_message_metatype(mt.arg, &msg))) {
			mpt_output_log(cl->out, __func__, MPT_FCNLOG(Error), "%s",
			               MPT_tr("failed to create argument stream"));
			return -1;
		}
	}
	/* prepare client */
	res = cl->_vptr->prep(cl, src);
	if (src) src->_vptr->unref(src);
	
	if (res < 0) {
		return MPT_event_fail(ev, MPT_tr("client preparation failed"));
	}
	return MPT_event_good(ev, MPT_tr("client preparation completed"));
}
