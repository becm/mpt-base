/*!
 * finalize connection data
 */

#include <errno.h>

#include "array.h"
#include "queue.h"
#include "event.h"

#include "message.h"

#include "output.h"

/*!
 * \ingroup mptOutput
 * \brief interactive message for connection
 * 
 * Register new interactive message on connection data.
 * 
 * Only available if no message is composed at the moment.
 * 
 * The next new message will use the registered ID.
 * 
 * \param con   connection descriptor
 * \param ctl   function to call on answer
 * \param udata user argument to callback function
 * 
 * \return state of property
 */
extern int mpt_connection_await(MPT_STRUCT(connection) *con, int (*ctl)(void *, const MPT_STRUCT(message) *), void *udata)
{
	MPT_STRUCT(command) *cmd;
	size_t max;
	
	/* no socket or stream open */
	if (!MPT_socket_active(&con->out.sock) && !(con->out.buf._buf)) {
		return MPT_ERROR(BadArgument);
	}
	/* message in progress */
	if (con->cid || (con->out.state & MPT_OUTFLAG(Active))) {
		return MPT_ERROR(BadOperation);
	}
	if ((max = con->out._idlen) > sizeof(con->cid)) {
		max = sizeof(con->cid);
	}
	if (!(cmd = mpt_command_nextid(&con->_wait, max))) {
		return MPT_ERROR(BadValue);
	}
	/* make next message non-local */
	con->out.state |= MPT_OUTFLAG(Remote);
	con->cid = cmd->id;
	
	if (ctl) {
		cmd->cmd = (int (*)()) ctl;
		cmd->arg = udata;
	}
	return 1 + cmd - ((MPT_STRUCT(command) *) (con->_wait._buf + 1));
}
