/*!
 * finalize connection data
 */

#include <stdio.h>
#include <string.h>

#include "array.h"
#include "queue.h"
#include "event.h"

#include "message.h"

#include "output.h"

/*!
 * \ingroup mptOutput
 * \brief log to connection
 * 
 * Push log message to connection or standard output
 * if message is composed or forwarding is disabled.
 * 
 * \param con   connection descriptor
 * \param from  log source
 * \param type  log message type
 * \param fmt   printf-style text format
 * \param va    arguments list
 * 
 * \return type of operation
 */
extern int mpt_connection_log(MPT_STRUCT(connection) *con, const char *from, int type, const char *msg)
{
	MPT_STRUCT(msgtype) hdr;
	ssize_t len;
	
	/* send log entry to contact */
	if ((con->out.state & MPT_ENUM(OutputActive))) {
		return MPT_ERROR(MessageInProgress);
	}
	hdr.cmd = MPT_ENUM(MessageOutput);
	hdr.arg = type & 0xff;
	
	len = mpt_connection_push(con, sizeof(hdr), &hdr);
	if (len <= 0) {
		return len;
	}
	if (len < 1) {
		mpt_connection_push(con, 1, 0);
		return MPT_ERROR(MissingBuffer);
	}
	if (from) {
		mpt_connection_push(con, strlen(from), from);
	}
	if (mpt_connection_push(con, 1, "") < 1) {
		mpt_connection_push(con, 1, 0);
		return MPT_ERROR(MissingBuffer);
	}
	if (msg && (len = strlen(msg))) {
		if (len <= MPT_OUTPUT_LOGMSG_MAX) {
			len = mpt_connection_push(con, len, msg);
		}
		/* indicate truncated message */
		else if ((len = mpt_connection_push(con, MPT_OUTPUT_LOGMSG_MAX - 1, msg)) >= 0) {
			len = mpt_connection_push(con, 1, "");
		}
		/* reset message state */
		if (len < 0) {
			mpt_connection_push(con, 1, 0);
			return len;
		}
	}
	mpt_connection_push(con, 0, 0);
	
	return 1;
}
