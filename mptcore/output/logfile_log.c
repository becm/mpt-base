/*!
 * log to file
 */

#include <stdio.h>

#include "message.h"
#include "convert.h"

#include "output.h"

/*!
 * \ingroup mptOutput
 * \brief log to file
 * 
 * Print message to logfile.
 * 
 * \param log   logfile descriptor
 * \param from  log data origin
 * \param type  severity and display flags
 * \param fmt   argument format
 * \param args  argument list
 * 
 * \return error or written lines
 */
extern int mpt_logfile_log(MPT_STRUCT(logfile) *log, const char *from, int type, const char *fmt, va_list args)
{
	const char *reset, *newline;
	FILE *fd;
	
	/* discard lower priority log message */
	if (log->ignore
	    && (type & 0x1f) >= log->ignore) {
		return 0;
	}
	/* message being composed */
	if (log->state & MPT_OUTFLAG(Active)) {
		return MPT_MESGERR(InProgress);
	}
	/* use log file */
	if ((type & MPT_LOG(File)) && (fd = log->file)) {
		newline = mpt_newline_string(log->lsep);
	}
	/* select target file */
	else {
		fd = (type & ~MPT_LOG(File)) ? stderr : stdout;
		newline = mpt_newline_string(0);
		/* force color for terminal output */
		if ((log->state & MPT_OUTFLAG(PrintColor))) {
			type |= MPT_ENUM(LogPretty);
		}
	}
	/* write intro */
	reset = mpt_log_intro(fd, type);
	
	/* write source info */
	if (from) {
		fputs(from, fd);
		if ((type & MPT_ENUM(LogFunction))) {
			fputc('(', fd);
			fputc(')', fd);
		}
		fputc(':', fd);
		fputc(' ', fd);
		/* no further coloring */
		if (reset && !(type & MPT_ENUM(LogANSIMore))) {
			fputs(from, fd);
			reset = 0;
		}
	}
	/* print message content */
	if (fmt) {
		vfprintf(fd, fmt, args);
	}
	if (reset) fputs(reset, fd);
	fputs(newline, fd);
	fflush(fd);
	
	return 1;
}
