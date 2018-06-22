/*!
 * MPT default client config operations
 */

#include <stdarg.h>

#include "output.h"

#include "meta.h"

__MPT_NAMESPACE_BEGIN

/*!
 * \ingroup mptMeta
 * \brief log to metatype
 * 
 * Use metatype as target for log message.
 * 
 * \param mt   metatype instance
 * \param src  originating location
 * \param type message type and flags
 * \param fmt  log arguments format string
 * \param va   argument list
 * 
 * \return lor operation result
 */
int mpt_meta_vlog(const MPT_INTERFACE(metatype) *mt, const char *src, int type, const char *fmt, va_list va)
{
	MPT_INTERFACE(output) *out;
	MPT_INTERFACE(logger) *log;
	
	if (!mt) {
		if (!(log  = mpt_log_default())) {
			return 0;
		}
		return log->_vptr->log(log, src, type, fmt, va);
	}
	log = 0;
	if (mt->_vptr->conv(mt, MPT_type_pointer(MPT_ENUM(TypeLogger)), &log) > 0) {
		if (!log) {
			return 0;
		}
		return log->_vptr->log(log, src, type, fmt, va);
	}
	out = 0;
	if (mt->_vptr->conv(mt, MPT_type_pointer(MPT_ENUM(TypeOutput)), &out) > 0) {
		if (!out) {
			return 0;
		}
		return mpt_output_vlog(out, src, type, fmt, va);
	}
	return MPT_ERROR(BadType);
}
/*!
 * \ingroup mptMeta
 * \brief log to metatype
 * 
 * Use metatype as target for log message.
 * 
 * \param mt   metatype instance
 * \param fcn  originating function
 * \param type message type and flags
 * \param fmt  log arguments format string
 * 
 * \return log operation result
 */
int mpt_meta_log(const MPT_INTERFACE(metatype) *mt, const char *fcn, int type, const char *fmt, ...)
{
	va_list va;
	MPT_INTERFACE(logger) *log = 0;
	int ret = 0;
	
	if (!mt && !(log  = mpt_log_default())) {
		return 0;
	}
	va_start(va, fmt);
	if (mt) {
		ret = mpt_meta_vlog(mt, fcn, type | MPT_ENUM(LogFunction) | MPT_ENUM(LogPretty), fmt, va);
	} else {
		ret = log->_vptr->log(log, fcn, type, fmt, va);
	}
	va_end(va);
	return ret;
}
__MPT_NAMESPACE_END
