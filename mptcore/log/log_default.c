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

#include "message.h"

static int logSkip  = MPT_ENUM(LogDebug);
static int logFlags = MPT_ENUM(LogPretty);

static void loggerUnref(MPT_INTERFACE(logger) *out)
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
	fputc('\n', fd);
	
	return ret;
}

static MPT_INTERFACE_VPTR(logger) _defaultLoggerVptr = { loggerUnref, loggerLog};
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
 * \brief default log instance
 * 
 * Get pointer to default log instance.
 * 
 * \return log operation result
 */
extern int mpt_log_default_set(int val)
{
	if (val < 0) {
		logSkip  = MPT_ENUM(LogDebug);
		logFlags = MPT_ENUM(LogPretty);
	}
	else {
		logSkip  = val & 0xff;
		logFlags = val & 0xff00;
	}
	return logSkip | logFlags;
}
