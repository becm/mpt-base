/*!
 * print arguments to output/error descriptors.
 */

#include <stdio.h>
#include <unistd.h>

#include "message.h"

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
	
	if (out) {
		err = out->_vptr->log(out, where, type, fmt, ap);
	}
	else {
		const char *prefix = 0;
		FILE *fd = stderr;
		
		if (!type) {
			fd = stdout;
			fputc('#', fd); fputc(' ', fd);
		}
		else if (where) {
			if (!(type & 0x80) && isatty(fileno(fd)) < 0 && (prefix = mpt_output_prefix(type))) {
				fputs(prefix, fd);
			}
			fputs(where, fd); fputs(": ", fd);
		}
		err = fmt ? vfprintf(fd, fmt, ap) : 0;
		if (prefix) {
			fputs(prefix, fd);
		}
		fputc('\n', fd);
	}
	va_end(ap);
	
	return err;
}

