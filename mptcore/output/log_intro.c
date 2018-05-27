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

#include "output.h"

/*!
 * \ingroup mptLog
 * \brief write intro
 * 
 * Print color and/or type code to file.
 * 
 * \param fd    argument format
 * \param type  message type and intro settings
 * 
 * \return ANSII terminal restore string
 */
extern const char *mpt_log_intro(FILE *fd, int type)
{
	const char *reset = 0;
	const char *desc = 0;
	
	/* change terminal color */
	if ((type & MPT_ENUM(LogSelect))
	    && isatty(fileno(fd))
	    && (reset = mpt_ansi_code(type))) {
		fputs(reset, fd);
		reset = mpt_ansi_reset();
	}
	/* no text intro */
	if (!(type & MPT_ENUM(LogPrefix))) {
		return reset;
	}
	/* print message type */
	if ((desc = mpt_log_identifier(type))) {
		fputc('[', fd);
		fputs(desc, fd);
		fputc(']', fd);
		fputc(' ', fd);
		if (reset && !(type & MPT_ENUM(LogANSIMore))) {
			fputs(reset, fd);
			reset = 0;
		}
	}
	/* no existing marking */
	if (!reset && !desc) {
		fputc('#', fd);
		fputc(' ', fd);
	}
	return reset;
}
