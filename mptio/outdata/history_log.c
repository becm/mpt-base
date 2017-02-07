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

static int logOutput(MPT_INTERFACE(output) *out, const char *from, int type, const char *fmt, va_list args)
{
	MPT_STRUCT(value) val;
	char buf[MPT_OUTPUT_LOGMSG_MAX];
	
	val.fmt = 0;
	val.ptr = 0;
	if (fmt) {
		int len = vsnprintf(buf, sizeof(buf), fmt, args);
		if (len > (int) sizeof(buf)) {
			buf[sizeof(buf) - 1] = 0; /* indicate truncation */
		}
		val.ptr = buf;
	}
	return out->_vptr->log(out, from, type, &val);
}
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
extern int mpt_history_log(MPT_STRUCT(history) *hist, const char *from, int type, const char *fmt, va_list args)
{
	const char *reset, *newline;
	FILE *fd;
	
	if (hist->pass && (hist->state & MPT_ENUM(OutputRemote))) {
		int ret = logOutput(hist->pass, from, type, fmt, args);
		if (ret >= 0) {
			hist->state &= ~MPT_ENUM(OutputRemote);
		}
		return ret;
	}
	if (hist->state & MPT_ENUM(OutputActive)) {
		return MPT_ERROR(MessageInProgress);
	}
	/* local processing of log entry */
	if (!type) {
		fd = stdout;
		newline = mpt_newline_string(0);
		
		fputc('#', fd);
		fputc(' ', fd);
		reset = 0;
	}
	else if (type & MPT_LOG(File)) {
		if ((fd = hist->file)) {
			newline = mpt_newline_string(hist->endl);
		}
		else if (hist->pass) {
			return logOutput(hist->pass, from, type, fmt, args);
		}
		else {
			fd = (type & 0x7f) ? stderr : stdout;
			newline = mpt_newline_string(0);
		}
	}
	else {
		if (mpt_outdata_type(type & 0x7f, hist->level & 0xf) <= 0) {
			return 0;
		}
		fd = (type & 0x7f) ? stderr : stdout;
		newline = mpt_newline_string(0);
		
		/* use default log config */
		if (!(type & MPT_ENUM(LogPretty))) {
			type |= MPT_ENUM(LogPrefix);
			if (hist->state & MPT_ENUM(OutputPrintColor)) {
				type |= MPT_ENUM(LogSelect);
			}
		}
		reset = mpt_log_intro(fd, type, from);
	}
	if (fmt) {
		vfprintf(fd, fmt, args);
	}
	if (reset) fputs(reset, fd);
	fputs(newline, fd);
	fflush(fd);
	
	return 1;
}
