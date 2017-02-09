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
	int8_t hdr[4];
	int flen, mlen, len;
	ssize_t ret;
	
	/* send log entry to contact */
	if ((con->out.state & MPT_OUTFLAG(Active))) {
		return MPT_ERROR(MessageInProgress);
	}
	hdr[0] = MPT_ENUM(MessageOutput);
	hdr[1] = type & 0x7f;
	
	len = 2;
	mlen = flen = 0;
	if (from) {
		flen = strlen(from);
		if (type & MPT_ENUM(LogFunction)) {
			hdr[len++] = 0x1; /* SOH */
		}
	}
	ret = MPT_OUTPUT_LOGMSG_MAX;
	if (msg) {
		mlen = strlen(msg);
		ret -= 2;
	}
	if (flen >= (ret - len)) {
		return MPT_ERROR(MissingBuffer);
	}
	if ((ret = mpt_connection_push(con, len, hdr)) < 0) {
		return ret;
	}
	len = MPT_OUTPUT_LOGMSG_MAX - len;
	if (from) {
		mpt_connection_push(con, flen, from);
		len -= flen;
	}
	if (msg) {
		*hdr = 0x2; /* STX */
		mpt_connection_push(con, 1, hdr);
		--len;
		
		if (len > mlen) {
			mpt_connection_push(con, mlen, msg);
			*hdr = 0x3; /* ETX */
			mpt_connection_push(con, 1, hdr);
		} else {
			mpt_connection_push(con, len, msg);
			len = 0;
		}
	}
	mpt_connection_push(con, 0, 0);
	
	return len ? 2 : 1;
}
