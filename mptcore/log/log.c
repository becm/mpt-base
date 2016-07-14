/*!
 * print arguments to output/error descriptors.
 */

#include "core.h"

/*!
 * \ingroup mptLog
 * \brief push log message
 * 
 * Push message to passed or default log descriptor.
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
	
	if (!out) out = mpt_log_default();
	err = out->_vptr->log(out, where, type | MPT_ENUM(LogPretty) | MPT_ENUM(LogFunction), fmt, ap);
	va_end(ap);
	
	return err;
}
