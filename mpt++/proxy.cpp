/*!
 * MPT default client config operations
 */

#include <stdio.h>

#include "message.h"

#include "client.h"

__MPT_NAMESPACE_BEGIN

/*!
 * \ingroup mptClient
 * \brief log to proxy
 * 
 * Select and use log target for message.
 * 
 * \param fcn  originating location
 * \param type message type and flags
 * \param fmt  log arguments format string
 * 
 * \return lor operation result
 */
int proxy::log(const char *fcn, int type, const char *fmt, ...) const
{
    va_list va;
    int ret = 0;
    if (fmt) va_start(va, fmt);
    class logger *log;
    class output *out;

    type |= logger::LogFunction;

    if ((log = logger.pointer())) {
        ret = log->log(fcn, type, fmt, va);
    }
    else if ((out = output.pointer())) {
        ret = mpt_output_log(out, fcn, type, fmt, va);
    }
    if (fmt) va_end(va);
    return ret;
}

__MPT_NAMESPACE_END
