/*!*
 * output interface to Qt
 */

#include <string.h>
#include <stdio.h>
#include <unistd.h>

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

class defaultLogger : public logger
{
public:
    int unref();
    int log(const char *, int, const char *, va_list );
};
int defaultLogger::unref()
{ return 0; }

int defaultLogger::log(const char *from, int arg, const char *fmt, va_list va)
{
    FILE *fd;
    if (arg) {
        fd = stderr;
        if (from) {
            fputs(from, fd); fputs(": ", fd);
        }
    } else {
        fd = stdout;
    }
    arg = vfprintf(fd, fmt, va);
    fputc('\n', fd);
    return arg;
}

logger *logger::defaultInstance()
{
    static defaultLogger log;
    return &log;
}


static Output &msgOut(Output &out, const char *fcn, const char *nspace)
{
    if (!fcn) return out;
    if (nspace && *nspace) out.nospace() << nspace << "::";
    out.nospace() << fcn << "[" << getpid() << "]:";
    return out;
}

Output debug(const char *fcn, const char *nspace)
{
    Output out(LogInfo);
    return msgOut(out, fcn, nspace).space().space();
}

Output warning(const char *fcn, const char *nspace)
{
    Output out(LogWarning);
    return msgOut(out, fcn, nspace).space();
}
Output critical(const char *fcn, const char *nspace)
{
    Output out(LogCritical);
    return msgOut(out, fcn, nspace).space();
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
    if (type >= 0 && (o = logger::defaultInstance())) {
        const std::string &str = buf.str();
        mpt_log(o, 0, type, "%s", str.c_str());
    }
    delete this;
    return 0;
}

__MPT_NAMESPACE_END
