/*!*
 * output interface to Qt
 */

#include <limits>
#include <cstdio>

#include <unistd.h>

#include "array.h"

#include "object.h"

__MPT_NAMESPACE_BEGIN

// logger interfaces
int debug(const char *from, const char *fmt, ... )
{
    logger *log = logger::defaultInstance();
    if (!log) return 0;
    va_list va;
    if (fmt) va_start(va, fmt);
    int ret = log->log(from, log->Debug, fmt, va);
    if (fmt) va_end(va);
    return ret;
}
int warning(const char *from, const char *fmt, ... )
{
    logger *log = logger::defaultInstance();
    if (!log) return 0;
    va_list va;
    if (fmt) va_start(va, fmt);
    int ret = log->log(from, log->Warning, fmt, va);
    if (fmt) va_end(va);
    return ret;
}
int error(const char *from, const char *fmt, ... )
{
    logger *log = logger::defaultInstance();
    if (!log) return 0;
    va_list va;
    if (fmt) va_start(va, fmt);
    int ret = log->log(from, log->Error, fmt, va);
    if (fmt) va_end(va);
    return ret;
}
int critical(const char *from, const char *fmt, ... )
{
    logger *log = logger::defaultInstance();
    if (!log) return 0;
    va_list va;
    if (fmt) va_start(va, fmt);
    int ret = log->log(from, log->Critical, fmt, va);
    if (fmt) va_end(va);
    return ret;
}
int log(const char *fmt, ... )
{
    logger *log = logger::defaultInstance();
    if (!log) return 0;
    va_list va;
    if (fmt) va_start(va, fmt);
    int ret = log->log(0, LogMessage, fmt, va);
    if (fmt) va_end(va);
    return ret;
}
logger *logger::defaultInstance()
{
    return mpt_log_default(0, 0);
}
int logger::message(const char *from, int err, const char *fmt, ...)
{
    va_list va;
    if (fmt) va_start(va, fmt);
    int ret = log(from, err, fmt, va);
    if (fmt) va_end(va);
    return ret;
}

// logging store
int LogEntry::type() const
{
    Header *h;

    if (length() < sizeof(*h)) {
        return -2;
    }
    h = (Header *) base();
    return h->type;
}
const char *LogEntry::source() const
{
    Header *h;

    if (length() < (sizeof(*h)+1)) {
        return 0;
    }
    h = (Header *) base();

    if (!h->from) {
        return 0;
    }
    return (const char *) (h+1);
}
Slice<const char> LogEntry::data(int part) const
{
    Header *h = (Header *) base();
    const char *data;
    size_t skip, len = length();

    if ((len < sizeof(*h)) || ((len <= (skip = h->from + sizeof(*h))))) {
        return Slice<const char>(0, 0);
    }
    data = ((char *) h) + skip;
    len -= skip;

    while (part >= 0) {
        const char *end;

        if (!(end = (const char *) memchr(data, 0, len))) {
            if (part) {
                return Slice<const char>(0, 0);
            }
            break;
        }
        skip = end - data;
        if (!part) {
            len = skip;
            break;
        }
        ++skip;

        data += skip;
        len  -= skip;
    }
    return Slice<const char>(data, len);
}


int LogEntry::set(const char *from, int type, const char *fmt, va_list arg)
{
    Header *h;
    array d;
    size_t len;

    if (!(h = (Header *) d.append(sizeof(*h)))) {
        return -1;
    }
    h->args = 0;
    h->from = 0;
    h->type = type;
    h->_cmd = 0;

    static const size_t maxlen = std::numeric_limits<__decltype(h->from)>::max();
    if (!from) {
        len = 0;
    }
    else if ((len = strlen(from)) >= maxlen) {
        static const char end = 0;
        if (!(d.append(maxlen, from)) || !(d.append(1, &end))) {
            return -1;
        }
        len = maxlen;
    } else {
        if (!(d.append(++len, from))) {
            return -1;
        }
    }
    h = (Header *) d.base();
    h->from = len;

    if (!fmt) {
        h->args = 0;
    }
    else if (mpt_vprintf(&d, fmt, arg) < 0) {
        return -1;
    } else {
        h = (Header *) d.base();
        h->args = 1;
    }

    mpt_array_clone(this, &d);

    return length();
}

LogStore::LogStore(logger *next) : _next(next), _act(0), _flags(FlowNormal), _ignore(LogDebug), _level(0)
{ }
LogStore::~LogStore()
{ }
void LogStore::unref()
{
    delete this;
}
int LogStore::log(const char *from, int type, const char *fmt, va_list arg)
{
    int save = 0, pass = 0, code = 0x7f & type;

    if (!code) {
        save |= _flags & SaveMessage;
        pass |= _flags & PassMessage;
    }
    else if (code >= _ignore) {
        save = (type & LogFile) && (_flags & SaveLogAll);
        pass |= _flags & PassUnsaved;
    } else {
        save = 1;
        pass |= _flags & PassSaved;
    }
    if (type & LogFile) {
        pass |= _flags & PassFile;
    }
    if (_next && pass) {
        va_list tmp;
        va_copy(tmp, arg);
        _next->log(from, type, fmt, tmp);
        va_end(tmp);
    }
    if (!save) {
        return 0;
    }
    LogEntry m;
    int ret;

    if (code && code < _level) _level = code;

    if ((ret = m.set(from, type, fmt, arg)) < 0) {
        return ret;
    }
    if (!_msg.insert(_msg.length(), m)) {
        return -1;
    }
    return ret;
}

const LogEntry *LogStore::nextEntry()
{
    LogEntry *e;

    if ((e = _msg.get(_act))) {
        ++_act;
    }
    return e;
}

void LogStore::clearLog()
{
    _msg.clear();
    _act = 0;
    _level = 0;
}

bool LogStore::setIgnoreLevel(int val)
{
    if (val < 0) val = LogDebug;
    else if (val >= LogFile) return false;
    _ignore = val;
    return true;
}
bool LogStore::setFlowFlags(int val)
{
    _flags = val;
    return true;
}

__MPT_NAMESPACE_END
