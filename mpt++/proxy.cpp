/*!
 * MPT default client config operations
 */

#include "message.h"

#include "client.h"

__MPT_NAMESPACE_BEGIN

/*!
 * \ingroup mptClient
 * \brief log to client
 * 
 * Get global config element
 * 
 * \param p  element location
 * 
 * \return metatype at location
 */
int proxy::log(const char *fcn, int type, const char *fmt, ...) const
{
    va_list va;
    int ret = 0;
    if (fmt) va_start(va, fmt);
    class logger *log;
    class output *out;
    
    if ((log = logger.pointer())) {
        ret = log->log(fcn, type, fmt, va);
    }
    else if ((out = output.pointer())) {
        ret = out->log(fcn, type, fmt, va);
    }
    if (fmt) va_end(va);
    return ret;
}

__MPT_NAMESPACE_END
