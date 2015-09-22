/*!
 * print arguments to output/error descriptors.
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

#include "message.h"


/*!
 * \ingroup mptCore
 * \brief push log message
 * 
 * Push message to log descriptor
 * or stderr when null.
 * 
 * \param fd    argument format
 * \param type  message type
 * \param where log source
 * 
 * \return log operation result
 */
extern const char *mpt_log_start(FILE *fd, const char *where, int type)
{
	const char *ansi = 0;
	const char *desc = 0;
	size_t len;
	
	if ((type & MPT_ENUM(LogSelect))
	    && isatty(fileno(fd))
	    && (ansi = mpt_ansi_code(type))) {
		fputs(ansi, fd);
		ansi = mpt_ansi_reset();
		if (!where && !(type & MPT_ENUM(LogANSIMore))) {
			if (type < 0) type &= 0xff;
			type |= MPT_ENUM(LogPrefix);
		}
	}
	if ((type & MPT_ENUM(LogPrefix)) && (desc = mpt_message_identifier(type))) {
		fputc('[', fd);
		fputs(desc, fd);
		fputc(']', fd);
		fputc(' ', fd);
		if (ansi && (!where || !(type & MPT_ENUM(LogANSIMore)))) {
			fputs(ansi, fd);
			ansi = 0;
		}
	}
	if (!desc && !ansi && !(type & 0xff) && (type & MPT_ENUM(LogPrefix))) {
		fputc('#', fd);
		fputc(' ', fd);
	}
	if (where && (len = strlen(where))) {
		fwrite(where, len, 1, fd);
		if ((type & MPT_ENUM(LogFunction)) && isalpha(where[len-1])) {
			fputc('(', fd);
			fputc(')', fd);
		}
		if (ansi) {
			fputs(ansi, fd);
			ansi = 0;
		}
		fputs(": ", fd);
	}
	return ansi;
}

static int loggerUnref(MPT_INTERFACE(logger) *out)
{
	(void) out;
	return 1;
}
static FILE *loggerErr = 0;

static int loggerLog(MPT_INTERFACE(logger) *out, const char *where, int type, const char *fmt, va_list ap)
{
	const char *ansi = 0;
	FILE *fd;
	int ret;
	
	(void) out;
	
	if (!(type & 0xff)) {
		fd = stdout;
	} else {
		if (!(type & MPT_ENUM(LogPretty))) {
			type |= MPT_ENUM(LogPrefix) | MPT_ENUM(LogSelect);
		}
		fd = loggerErr ? loggerErr : stderr;
	}
	ansi = mpt_log_start(fd, where, type);
	ret = fmt ? vfprintf(fd, fmt, ap) : 0;
	if (ansi) fputs(ansi, fd);
	fputc('\n', fd);
	
	return ret;
}



static MPT_INTERFACE_VPTR(logger) _defaultLoggerVptr = { loggerUnref, loggerLog};
static MPT_INTERFACE(logger) defaultLogger = { &_defaultLoggerVptr };

/*!
 * \ingroup mptCore
 * \brief push log message
 * 
 * Push message to log descriptor
 * or stderr when null.
 * 
 * \param log   logging descriptor
 * \param where log source
 * \param type  message type
 * \param fmt   argument format
 * 
 * \return log operation result
 */
extern int mpt_log(MPT_INTERFACE(logger) *out, const char *where, int type, const char *fmt, ... )
{
	va_list ap;
	int err;
	
	va_start(ap, fmt);
	
	if (!out) out = &defaultLogger;
	err = out->_vptr->log(out, where, type, fmt, ap);
	va_end(ap);
	
	return err;
}

/*!
 * \ingroup mptCore
 * \brief default log instance
 * 
 * Get pointer to default log instance.
 * 
 * \return log operation result
 */
extern MPT_INTERFACE(logger) *_mpt_log_default(FILE *err)
{
	if (err) loggerErr = err;
	return &defaultLogger;
}
