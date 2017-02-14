/*!
 * infinite loop until events can't be received anymore.
 */

#include <poll.h>

#include "event.h"
#include "array.h"

#include "notify.h"

/*!
 * \ingroup mptNotify
 * \brief execute event loop
 * 
 * Process events from notifier context.
 * 
 * \param n  notification descriptor
 * 
 * \return state change or wait error
 */
extern int mpt_loop(MPT_STRUCT(notify) *n)
{
	int def = !n->_fdused && n->_disp.cmd ? 1 : 0;
	
	while (1) {
		MPT_INTERFACE(input) *in;
		int state;
		
		/* simple event processing */
		if ((in = mpt_notify_next(n))) {
			MPT_STRUCT(buffer) *s;
			
			/* command needs further processing */
			if ((state = in->_vptr->dispatch(in, n->_disp.cmd, n->_disp.arg)) < 0) {
				continue;
			}
			def = (state & MPT_EVENTFLAG(Default)) ? 1 : 0;
			
			if (state & MPT_EVENTFLAG(Terminate)) {
				return state;
			}
			if (!(state & MPT_EVENTFLAG(Retry))) {
				continue;
			}
			if ((s = n->_wait._buf) && s->used) {
				size_t len = mpt_array_compact((void **) (s+1), s->used/sizeof(in));
				s->used = len * sizeof(in);
			}
			mpt_array_append(&n->_wait, sizeof(in), &in);
			continue;
		}
		/* try default event */
		else if (def) {
			MPT_STRUCT(event) ev = MPT_EVENT_INIT;
			
			if ((state = mpt_notify_wait(n, POLLIN, 0)) > 0) {
				continue;
			}
			if (n->_disp.cmd
			    && (state = n->_disp.cmd(n->_disp.arg, &ev)) < 0) {
				continue;
			}
			def = (state & MPT_EVENTFLAG(Default)) ? 1 : 0;
			
			if (state & MPT_EVENTFLAG(Terminate)) {
				return state;
			}
			continue;
		}
		/* wait for further input */
		if ((state = mpt_notify_wait(n, -1, -1)) < 0) {
			return n->_fdused ? state : 0;
		}
	}
}

