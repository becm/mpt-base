/*!
 * finalize connection data
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "event.h"
#include "message.h"
#include "array.h"

#include "stream.h"
#include "convert.h"

#include "output.h"

/*!
 * \ingroup mptOutput
 * \brief push to history
 * 
 * Append data to history descriptor or pass to next output.
 * 
 * \param hist  history descriptor
 * \param from  log data origin
 * \param type  severity and display flags
 * \param fmt   argument format
 * \param args  argument list
 * 
 * \return error or written lines
 */
extern int mpt_history_log(MPT_STRUCT(histinfo) *hist, const char *from, int type, const char *fmt, va_list args)
{
	const char *reset, *newline;
	FILE *fd;
	
	/* message being composed */
	if (hist->state & MPT_OUTFLAG(Active)) {
		return MPT_ERROR(MessageInProgress);
	}
	/* use log file */
	if ((type & MPT_LOG(File)) && (fd = hist->file)) {
		newline = mpt_newline_string(hist->lsep);
	}
	/* select target file */
	else {
		fd = (type & ~MPT_LOG(File)) ? stderr : stdout;
		newline = mpt_newline_string(0);
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
