/*!
 * MPT default client config operations
 */

#include <stdarg.h>

#include "meta.h"

#include "message.h"
#include "output.h"

#include "client.h"

__MPT_NAMESPACE_BEGIN

/*!
 * \ingroup mptClient
 * \brief log to proxy
 * 
 * Select and use log conversion of reference for message.
 * 
 * \param pr   proxy descriptor
 * \param src  originating location
 * \param type message type and flags
 * \param fmt  log arguments format string
 * \param va   argument list
 * 
 * \return lor operation result
 */
int mpt_proxy_vlog(const MPT_STRUCT(proxy) *pr, const char *src, int type, const char *fmt, va_list va)
{
	MPT_INTERFACE(metatype) *mt;
	MPT_INTERFACE(output) *out;
	MPT_INTERFACE(logger) *log;
	
	if (!(mt = pr->_ref)) {
		return 0;
	}
	log = 0;
	if (mt->_vptr->conv(mt, MPT_ENUM(TypeLogger), &log) > 0) {
		if (!log && !(log = mpt_log_default())) {
			return 0;
		}
		return log->_vptr->log(log, src, type, fmt, va);
	}
	if (mt->_vptr->conv(mt, MPT_ENUM(TypeOutput), &out) > 0) {
		if (!out) {
			return 0;
		}
		return mpt_output_vlog(out, src, type, fmt, va);
	}
	return MPT_ERROR(BadType);
}
/*!
 * \ingroup mptClient
 * \brief log to proxy
 * 
 * Select and use log interface for message.
 * 
 * \param fcn  originating function
 * \param type message type and flags
 * \param fmt  log arguments format string
 * 
 * \return lor operation result
 */
int mpt_proxy_log(const MPT_STRUCT(proxy) *pr, const char *fcn, int type, const char *fmt, ...)
{
	va_list va;
	int ret;
	
	va_start(va, fmt);
	ret = mpt_proxy_vlog(pr, fcn, type | MPT_ENUM(LogFunction) | MPT_ENUM(LogPretty), fmt, va);
	va_end(va);
	return ret;
}
__MPT_NAMESPACE_END
