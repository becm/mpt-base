/*!*
 * output interface to Qt
 */

#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "message.h"

#include "object.h"

__MPT_NAMESPACE_BEGIN

template class Reference<Output::Message>;

// logger interfaces
int logger::warning(const char *from, const char *fmt, ... )
{
    va_list va;
    int ret;
    va_start(va, fmt);
    ret = log(from, LogWarning, fmt, va);
    va_end(va);
    return ret;
}
int logger::error(const char *from, const char *fmt, ... )
{
    va_list va;
    int ret;
    va_start(va, fmt);
    ret = log(from, LogError, fmt, va);
    va_end(va);
    return ret;
}
int logger::critical(const char *from, const char *fmt, ... )
{
    va_list va;
    int ret;
    va_start(va, fmt);
    ret = log(from, LogCritical, fmt, va);
    va_end(va);
    return ret;
}
logger *logger::defaultInstance()
{
    return _mpt_log_default(stderr);
}


bool Output::setSource(const char *fcn, const char *nspace, pid_t pid)
{
    Message *msg = _msg;
    if (msg->_flen) return false;
    if (nspace && *nspace) msg->buf << nspace << "::";
    
    if (pid < 0) pid = getpid();
    
    msg->buf << fcn;
    if (pid) msg->buf << '[' << pid << ']';
    msg->buf.put(0);
    msg->_flen = msg->buf.str().size();
    return true;
}

Output debug(const char *fcn, const char *nspace)
{
    Output out(LogDebug);
    out.setSource(fcn, nspace);
    return out;
}

Output warning(const char *fcn, const char *nspace)
{
    Output out(LogWarning);
    out.setSource(fcn, nspace);
    return out;
}
Output critical(const char *fcn, const char *nspace)
{
    Output out(LogCritical);
    out.setSource(fcn, nspace);
    return out;
}

Output::Message *Output::Message::addref()
{
    if (++_ref) return this;
    --_ref;
    return 0;
}
int Output::Message::unref()
{
    if (!_ref) return 0;
    if (--_ref) return _ref;
    logger *o;
    if ((o = _out) || (o = logger::defaultInstance())) {
        const char *msg = buf.str().c_str(), *fcn = 0;
        if (_flen) {
            fcn = msg;
            msg += _flen;
        }
        mpt_log(o, fcn, _type, "%s", msg);
    }
    if (_out) _out->unref();
    delete this;
    return 0;
}

__MPT_NAMESPACE_END
