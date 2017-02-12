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
 * Select and use log interface for message.
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
    va_start(va, fmt);
    int ret = mpt_proxy_vlog(this, fcn, type | logger::LogFunction, fmt, va);
    va_end(va);
    return ret;
}
__MPT_NAMESPACE_END
