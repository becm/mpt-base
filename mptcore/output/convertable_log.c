/*!
 * MPT default client config operations
 */

#include <stdarg.h>

#include "types.h"

#include "output.h"

__MPT_NAMESPACE_BEGIN

/*!
 * \ingroup mptOutput
 * \brief log to convertable
 * 
 * Use convertable as target for log message.
 * 
 * \param dest metatype instance
 * \param src  originating location
 * \param type message type and flags
 * \param fmt  log arguments format string
 * \param va   argument list
 * 
 * \return lor operation result
 */
int mpt_convertable_vlog(MPT_INTERFACE(convertable) *dest, const char *src, int type, const char *fmt, va_list va)
{
	MPT_INTERFACE(output) *out;
	MPT_INTERFACE(logger) *log;
	
	if (!dest) {
		if (!(log  = mpt_log_default())) {
			return 0;
		}
		return log->_vptr->log(log, src, type, fmt, va);
	}
	log = 0;
	if (dest->_vptr->convert(dest, MPT_ENUM(TypeLoggerPtr), &log) > 0) {
		if (!log) {
			return 0;
		}
		return log->_vptr->log(log, src, type, fmt, va);
	}
	out = 0;
	if (dest->_vptr->convert(dest, MPT_ENUM(TypeOutputPtr), &out) > 0) {
		if (!out) {
			return 0;
		}
		return mpt_output_vlog(out, src, type, fmt, va);
	}
	return MPT_ERROR(BadType);
}
/*!
 * \ingroup mptOutput
 * \brief log to convertable
 * 
 * Use convertable as target for log message.
 * 
 * \param dest metatype instance
 * \param fcn  originating function
 * \param type message type and flags
 * \param fmt  log arguments format string
 * 
 * \return log operation result
 */
int mpt_convertable_log(MPT_INTERFACE(convertable) *dest, const char *fcn, int type, const char *fmt, ...)
{
	va_list va;
	MPT_INTERFACE(logger) *log = 0;
	int ret = 0;
	
	if (!dest && !(log  = mpt_log_default())) {
		return 0;
	}
	va_start(va, fmt);
	if (dest) {
		ret = mpt_convertable_vlog(dest, fcn, type | MPT_ENUM(LogFunction) | MPT_ENUM(LogPretty), fmt, va);
	} else {
		ret = log->_vptr->log(log, fcn, type, fmt, va);
	}
	va_end(va);
	return ret;
}
__MPT_NAMESPACE_END
