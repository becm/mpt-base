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
 * \param con      connection descriptor
 * \param timeout  time to wait for return messages
 * 
 * \return state of property
 */
extern int mpt_connection_await(MPT_STRUCT(connection) *con, int (*ctl)(void *, const MPT_STRUCT(message) *), void *udata)
{
	MPT_STRUCT(command) *cmd;
	
	if (!MPT_socket_active(&con->out.sock) || !(con->out._buf)) {
		return MPT_ERROR(BadArgument);
	}
	/* message in progress */
	if (con->cid || (con->out.state & MPT_ENUM(OutputActive))) {
		return MPT_ERROR(BadOperation);
	}
	if (!(cmd = mpt_command_nextid(&con->_wait, con->out._idlen))) {
		return MPT_ERROR(BadValue);
	}
	/* make next message non-local */
	con->out.state |= MPT_ENUM(OutputRemote);
	con->cid = cmd->id;
	
	if (ctl) {
		cmd->cmd = (int (*)()) ctl;
		cmd->arg = udata;
	}
	return 1 + cmd - ((MPT_STRUCT(command) *) (con->_wait._buf + 1));
}
