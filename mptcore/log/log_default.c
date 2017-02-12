/*!
 * print log to stdout/stderr descriptors.
 */

#ifndef _POSIX_C_SOURCE
# define _POSIX_C_SOURCE 200809L
#endif


#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

#include "convert.h"

#include "message.h"

static int logSkip  = MPT_LOG(Info);
static int logFlags = MPT_ENUM(LogPretty);

static void loggerUnref(MPT_INTERFACE(unrefable) *out)
{
	(void) out;
}

static int loggerLog(MPT_INTERFACE(logger) *out, const char *where, int type, const char *fmt, va_list ap)
{
	const char *ansi = 0;
	FILE *fd;
	int ret;
	
	(void) out;
	
	if ((type & 0x3f) >= logSkip) {
		return 0;
	}
	type |= logFlags;
	/* select target file descriptor */
	fd = (type & 0xff) ? stderr : stdout;
	
	ansi = mpt_log_intro(fd, type);
	if (where) {
		fputs(where, fd);
		if (type & MPT_ENUM(LogFunction)) {
			fputc('(', fd);
			fputc(')', fd);
		}
		fputc(':', fd);
		fputc(' ', fd);
		if (ansi) {
			fputs(ansi, fd);
			ansi = 0;
		}
	}
	ret = fmt ? vfprintf(fd, fmt, ap) : 0;
	if (ansi) fputs(ansi, fd);
	fputs(mpt_newline_string(0), fd);
	
	return ret;
}

static MPT_INTERFACE_VPTR(logger) _defaultLoggerVptr = { { loggerUnref }, loggerLog};
static MPT_INTERFACE(logger) defaultLogger = { &_defaultLoggerVptr };

/*!
 * \ingroup mptLog
 * \brief default log instance
 * 
 * Get pointer to default log instance.
 * 
 * \return log operation result
 */
extern MPT_INTERFACE(logger) *mpt_log_default()
{
	return &defaultLogger;
}
/*!
 * \ingroup mptLog
 * \brief set log format
 * 
 * Set flags for log colour and descriptor.
 * 
 * \return default log flags
 */
extern int mpt_log_default_format(int val)
{
	if (val < 0) {
		logFlags = MPT_ENUM(LogPretty);
	} else {
		logFlags = val & (MPT_ENUM(LogPretty) | MPT_LOG(File));
	}
	return logSkip | logFlags;
}
/*!
 * \ingroup mptLog
 * \brief set skip limit
 * 
 * Set lowest log value to skip.
 * 
 * \return default log flags
 */
extern int mpt_log_default_skip(int val)
{
	if (val < 0) {
		logSkip = MPT_LOG(Info);
	}else {
		logSkip = val & 0xff;
	}
	return logSkip | logFlags;
}
