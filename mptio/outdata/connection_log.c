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
extern int mpt_connection_log(MPT_STRUCT(connection) *con, const char *from, int type, const char *fmt, va_list va)
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
	if (fmt) {
		char buf[1024];
		int plen;
		
		plen = vsnprintf(buf, sizeof(buf), fmt, va);
		
		/* zero termination indicates truncation,
		 * just ignore Microsoft's fucked up version without termination */
		if (plen >= (int) sizeof(buf)) plen = sizeof(buf);
		
		if (plen > 0 && (len = mpt_connection_push(con, plen, buf)) < 0) {
			mpt_connection_push(con, 1, 0);
			return len;
		}
	}
	mpt_connection_push(con, 0, 0);
	
	return 1;
}
