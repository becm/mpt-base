/*!
 * init/fini MPT dispatch descriptor.
 */

#include <unistd.h>

#include "array.h"
#include "message.h"
#include "event.h"

static int unknownEvent(void *arg, MPT_STRUCT(event) *ev)
{
	MPT_INTERFACE(dispatch) *disp = arg;
	MPT_STRUCT(msgtype) mt = MPT_MSGTYPE_INIT;
	
	if (!ev) return 0;
	
	if (ev->msg) {
		MPT_STRUCT(message) msg = *ev->msg;
		
		if (mpt_message_read(&msg, sizeof(mt), &mt) < 2) {
			mpt_event_reply(ev, MPT_ENUM(LogError), MPT_tr("incomplete message"));
			return -2;
		}
	}
	if (mpt_event_reply(ev, -1, MPT_tr("invalid message command")) >= 0) {
		return MPT_ENUM(EventFail);
	}
	mpt_output_log(disp ? disp->_out : 0, "mpt::event::unknown", MPT_ENUM(LogError), "%s",
	               MPT_tr("invalid command"));
	
	ev->id = 0;
	return MPT_ENUM(EventDefault);
}

/*!
 * \ingroup mptEvent
 * \brief initialize dispatch
 * 
 * Set initial values and reserve system resources.
 * 
 * \param disp  dispatch descriptor
 */
extern void mpt_dispatch_init(MPT_STRUCT(dispatch) *disp)
{
	static const MPT_STRUCT(array) def;
	disp->_out = 0;
	
	disp->_cmd = def;
	
	disp->_err.cmd = unknownEvent;
	disp->_err.arg = disp;
}

/*!
 * \ingroup mptEvent
 * \brief close dispatch
 * 
 * Clear bound references and system resources.
 * 
 * \param disp  dispatch descriptor
 */
extern void mpt_dispatch_fini(MPT_STRUCT(dispatch) *disp)
{
	if (disp->_out) {
		disp->_out->_vptr->_mt.unref((void *) disp->_out);
		disp->_out = 0;
	}
	/* dereference registered commands */
	disp->_def  = 0;
	mpt_command_clear(&disp->_cmd);
	mpt_array_clone(&disp->_cmd, 0);
	
	if (disp->_err.cmd) {
		disp->_err.cmd(disp->_err.arg, 0);
		disp->_err.cmd = 0;
		disp->_err.arg = 0;
	}
}
