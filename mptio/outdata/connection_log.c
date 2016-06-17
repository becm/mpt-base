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
	FILE *fd;
	const char *ansi = 0;
	
	/* send log entry to contact */
	if (!(con->out.state & MPT_ENUM(OutputActive))
	    && (con->out.state & MPT_ENUM(OutputRemote))) {
		MPT_STRUCT(msgtype) hdr;
		
		hdr.cmd = MPT_ENUM(MessageOutput);
		hdr.arg = type & 0xff;
		
		mpt_connection_push(con, sizeof(hdr), &hdr);
		
		if (type && from) {
			mpt_connection_push(con, strlen(from)+1, from);
		}
		if (fmt) {
			char buf[1024];
			int plen;
			
			plen = vsnprintf(buf, sizeof(buf), fmt, va);
			
			/* zero termination indicates truncation,
			 * just ignore Microsoft's fucked up version without termination */
			if (plen >= (int) sizeof(buf)) plen = sizeof(buf);
			
			if (plen > 0 && (plen = mpt_connection_push(con, plen, buf)) < 0) {
				mpt_connection_push(con, 1, 0);
				return plen;
			}
		}
		mpt_connection_push(con, 0, 0);
		
		return 2;
	}
	/* local processing of log entry */
	if (!(type & 0xff)) {
		type &= ~MPT_ENUM(LogPretty);
		fd = stdout;
		fputc('#', fd);
		fputc(' ', fd);
	}
	else {
		fd = stderr;
		if ((type & MPT_ENUM(LogFile)) && con->hist.file) {
			fd = con->hist.file;
			type &= ~MPT_ENUM(LogFile);
		}
		else if (mpt_output_file(type & 0x7f, con->out.level & 0xf) <= 0) {
			return 0;
		}
		/* use default log config */
		if (!(type & MPT_ENUM(LogPretty))) {
			type |= MPT_ENUM(LogPrefix);
			if (con->out.state & MPT_ENUM(OutputPrintColor)) {
				type |= MPT_ENUM(LogSelect);
			}
		}
	}
	ansi = mpt_log_intro(fd, type, from);
	if (fmt) {
		vfprintf(fd, fmt, va);
	}
	if (ansi) fputs(ansi, fd);
	fputc('\n', fd);
	fflush(fd);
	
	return 1;
}
