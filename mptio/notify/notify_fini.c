/*!
 * init/fini notifier.
 */

#include <unistd.h>

#include "notify.h"

/*!
 * \ingroup mptNotify
 * \brief close notification descriptor
 * 
 * Clear bound references and system resources.
 * 
 * \param no  notification descriptor
 */
extern void mpt_notify_fini(MPT_STRUCT(notify) *no)
{
	/* clear event handler */
	if (no->_disp.cmd) {
		no->_disp.cmd(no->_disp.arg, 0);
		no->_disp.cmd = 0;
		no->_disp.arg = 0;
	}
	/* clear temporal data */
	mpt_array_clone(&no->_wait, 0);
	/* clear slot data */
	mpt_array_clone(&no->_slot, 0);
	
	if (no->_sysfd >= 0) {
		(void) close(no->_sysfd);
	}
	no->_fdused = 0;
	no->_sysfd = -1;
}
