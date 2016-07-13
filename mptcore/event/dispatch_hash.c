/*!
 * call event for convert string message argument hash.
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <stdio.h>
#include <signal.h>

#include <sys/uio.h>

#include "array.h"
#include "config.h"
#include "message.h"
#include "event.h"

/*!
 * \ingroup mptEvent
 * \brief call handler for matching command
 * 
 * Determine command from message text initial argument.
 * Use hash to find matching registerd command.
 * 
 * \param disp  notification descriptor
 * \param ev   event data to dispatch
 * 
 * \return result of executed command
 */
extern int mpt_dispatch_hash(MPT_STRUCT(dispatch) *disp, MPT_STRUCT(event) *ev)
{
	MPT_STRUCT(message) msg;
	MPT_STRUCT(msgtype) mt = MPT_MSGTYPE_INIT;
	MPT_STRUCT(command) *cmd;
	ssize_t	len;
	
	if (!ev) {
		return 0;
	}
	if (!ev->msg) {
		return MPT_event_fail(ev, MPT_ERROR(MissingData), MPT_tr("missing message data"));
	}
	msg = *ev->msg;
	
	if ((len = mpt_message_read(&msg, sizeof(mt), &mt)) < (ssize_t) sizeof(mt)) {
		if (len) return MPT_event_fail(ev, MPT_ERROR(MissingData), MPT_tr("missing message type"));
		return MPT_event_fail(ev, MPT_ERROR(MissingData), MPT_tr("missing message header"));
	}
	if (mt.cmd != MPT_ENUM(MessageCommand)) {
		mt.arg = 0;
	}
	/* get message command (string) */
	if ((len = mpt_message_argv(&msg, mt.arg)) <= 0) {
		return MPT_event_fail(ev, len, MPT_tr("unable to get text command"));
	}
	/* continous data */
	if (msg.used >= (size_t) len) {
		if (!mt.arg && !((const char *) msg.base)[len-1]) {
			--len;
		}
		ev->id = mpt_hash(msg.base, len);
	}
	/* need aligned data */
	else {
		char buf[128];
		if ((size_t) len > sizeof(buf)) {
			return MPT_event_fail(ev, MPT_ERROR(MissingBuffer), MPT_tr("large unaligned text command"));
		}
		if (mpt_message_read(&msg, len, buf) != (size_t) len) {
			MPT_ABORT("conflicting message length");
		}
		if (!mt.arg && !buf[len-1]) {
			--len;
		}
		ev->id = mpt_hash(buf, len);
	}
	/* execute matching command */
	if ((cmd = mpt_command_get(&disp->_d, ev->id))) {
		if ((len = cmd->cmd(cmd->arg, ev)) < 0) {
			return MPT_event_fail(ev, len, MPT_tr("failed to execute command"));
		}
		return len;
	}
	if (mpt_config_get(0, "mpt.debug", '.', 0)) {
		fprintf(stderr, "%s: %s: %s,%d\n", "hash not found", __func__, __FILE__, __LINE__);
		raise(SIGSTOP);
	}
	/* execute fallback command */
	if (disp->_err.cmd) {
		return disp->_err.cmd(disp->_err.arg, ev);
	}
	return MPT_event_fail(ev, MPT_ERROR(BadValue), MPT_tr("unable to find corresponding command"));
}

