/*!
 * graphic event for dispatch output
 */

#include "array.h"
#include "meta.h"
#include "output.h"
#include "message.h"

#include "event.h"

static int clientOutput(void *ptr, MPT_STRUCT(event) *ev)
{
	MPT_INTERFACE(metatype) *mt = ptr;
	if (!ev) {
		mt->_vptr->ref.unref((void *) mt);
		return 0;
	}
	if (ev->msg) {
		MPT_STRUCT(message) msg = *ev->msg;
		MPT_STRUCT(msgtype) hdr;
		ssize_t part;
		int err;
		
		/* get command type */
		if ((part = mpt_message_read(&msg, sizeof(hdr), &hdr)) < (ssize_t) sizeof(hdr)) {
			return MPT_event_fail(ev, MPT_ERROR(MissingData), MPT_tr("missing message type"));
		}
		if (hdr.cmd != MPT_MESGTYPE(Command)) {
			return MPT_event_fail(ev, MPT_ERROR(BadArgument), MPT_tr("bad message type"));
		}
		/* consume command */
		if (((part = mpt_message_argv(&msg, hdr.arg)) <= 0)
		    || (mpt_message_read(&msg, part, 0) < (size_t) part)) {
			return MPT_event_fail(ev, MPT_ERROR(MissingData), MPT_tr("missing filter element"));
		}
		if ((err = mpt_output_control(mt, hdr.arg, &msg, mpt_log_default())) >= 0) {
			return MPT_event_good(ev, MPT_tr("output command processed"));
		}
		if (err == MPT_ERROR(MissingData)) {
			return MPT_event_fail(ev, err, MPT_tr("missing message content"));
		}
		if (err == MPT_ERROR(BadArgument)) {
			return MPT_event_fail(ev, err, MPT_tr("bad message type"));
		}
		if (err == MPT_ERROR(MissingBuffer)) {
			return MPT_event_fail(ev, err, MPT_tr("message buffer too small"));
		}
		if (err == MPT_ERROR(BadOperation)) {
			return MPT_event_fail(ev, err, MPT_tr("output operation failed"));
		}
		return MPT_event_fail(ev, err, MPT_tr("message processing error"));
	}
	return MPT_event_good(ev, MPT_tr("no output operation"));
}

/*!
 * \ingroup mptClient
 * \brief register graphic event
 * 
 * Set graphic handler for dispatch output.
 * 
 * \param dsp  dispatch descriptor
 * 
 * \retval 0  success
 * \retval <0 assignment error
 */
extern int mpt_dispatch_control(MPT_STRUCT(dispatch) *dsp, uintptr_t id, MPT_INTERFACE(metatype) *mt)
{
	if (!mt) {
		return mpt_dispatch_set(dsp, id, 0, 0);
	}
	if (mpt_dispatch_set(dsp, id, clientOutput, mt) < 0) {
		return MPT_ERROR(BadOperation);
	}
	return 0;
}
