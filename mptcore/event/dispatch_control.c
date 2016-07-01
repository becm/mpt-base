/*!
 * graphic event for dispatch output
 */

#include <string.h>

#include "array.h"
#include "output.h"
#include "message.h"

#include "event.h"

static int clientOutput(void *ptr, MPT_STRUCT(event) *ev)
{
	MPT_INTERFACE(output) *out = ptr;
	if (!ev) {
		out->_vptr->obj.unref((void *) out);
		return 0;
	}
	if (ev->msg) {
		MPT_STRUCT(message) msg = *ev->msg;
		MPT_STRUCT(msgtype) mt;
		ssize_t part;
		int err;
		
		/* get command type */
		if ((part = mpt_message_read(&msg, sizeof(mt), &mt)) < (ssize_t) sizeof(mt)) {
			return MPT_event_fail(ev, MPT_ERROR(MissingData), MPT_tr("missing message type"));
		}
		if (mt.cmd != MPT_ENUM(MessageCommand)) {
			return MPT_event_fail(ev, MPT_ERROR(BadArgument), MPT_tr("bad message type"));
		}
		/* consume command */
		if (((part = mpt_message_argv(&msg, mt.arg)) <= 0)
		    || (mpt_message_read(&msg, part, 0) < (size_t) part)) {
			return MPT_event_fail(ev, MPT_ERROR(MissingData), MPT_tr("missing dispatch wrapper"));
		}
		if ((err = mpt_output_control(out, mt.arg, &msg)) >= 0) {
			return MPT_event_good(ev, MPT_tr("graphic command processed"));
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
			return MPT_event_fail(ev, err, MPT_tr("grapgic operation failed"));
		}
		return MPT_event_fail(ev, err, MPT_tr("message processing error"));
	}
	return MPT_event_good(ev, MPT_tr("skip graphic binding"));
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
extern int mpt_dispatch_control(MPT_STRUCT(dispatch) *dsp, const char *name)
{
	MPT_INTERFACE(output) *out;
	uintptr_t id;
	
	if (!dsp || !name) {
		return MPT_ERROR(BadArgument);
	}
	id = mpt_hash(name, strlen(name));
	
	if (!(out = dsp->_out)) {
		return mpt_dispatch_set(dsp, id, 0, 0);
	}
	if (!out->_vptr->obj.addref((void *) out)) {
		return MPT_ERROR(BadType);
	}
	if (mpt_dispatch_set(dsp, id, clientOutput, out) < 0) {
		out->_vptr->obj.unref((void *) out);
		return MPT_ERROR(BadOperation);
	}
	return 0;
}
