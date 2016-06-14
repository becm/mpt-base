/*!
 * graphic event for dispatch output
 */

#include "array.h"
#include "message.h"

#include "event.h"

static int clientGrapic(void *ptr, MPT_STRUCT(event) *ev)
{
	MPT_INTERFACE(output) *out = ptr;
	if (!ev) {
		out->_vptr->obj.unref((void *) out);
		return 0;
	}
	if (ev->msg) {
		int err;
		
		if ((err = mpt_output_graphic(out, ev->msg)) >= 0) {
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
extern int mpt_dispatch_graphic(MPT_STRUCT(dispatch) *dsp)
{
	MPT_INTERFACE(output) *out;
	uintptr_t id;
	
	if (!dsp) {
		return MPT_ERROR(BadArgument);
	}
	id = mpt_hash("graphic", 7);
	
	if (!(out = dsp->_out)) {
		return mpt_dispatch_set(dsp, id, 0, 0);
	}
	if (!out->_vptr->obj.addref((void *) out)) {
		return MPT_ERROR(BadType);
	}
	if (mpt_dispatch_set(dsp, id, clientGrapic, out) < 0) {
		out->_vptr->obj.unref((void *) out);
		return MPT_ERROR(BadOperation);
	}
	return 0;
}
