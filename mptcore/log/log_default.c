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

static int logSkip  = MPT_LOG(Debug);
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
	
	if (!(type & 0xff)) {
		fd = stdout;
	} else if ((type &0x7f) >= logSkip) {
		return 0;
	}
	else {
		if (!(type & MPT_ENUM(LogPretty))) {
			type |= MPT_ENUM(LogPrefix) | MPT_ENUM(LogSelect);
		}
		fd = stderr;
	}
	ansi = mpt_log_intro(fd, type | logFlags, where);
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
		logFlags = val & MPT_ENUM(LogPretty);
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
		logSkip = MPT_LOG(Debug);
	}else {
		logSkip = val & 0xff;
	}
	return logSkip | logFlags;
}
