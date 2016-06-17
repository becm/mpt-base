/*!
 * print log line intro to file descriptor.
 */

#ifndef _POSIX_C_SOURCE
# define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

#include "core.h"


/*!
 * \ingroup mptLog
 * \brief push log message
 * 
 * Print color, type code and source (if given) to file.
 * 
 * color + color  -> color code
 * color + source -> color source
 * color -> color full line
 * 
 * \param fd    argument format
 * \param type  message type and intro settings
 * \param where log source
 * 
 * \return ANSII terminal restore string
 */
extern const char *mpt_log_intro(FILE *fd, int type, const char *where)
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
	if ((type & MPT_ENUM(LogPrefix)) && (desc = mpt_log_identifier(type))) {
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
