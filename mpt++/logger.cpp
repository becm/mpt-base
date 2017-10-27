/*!*
 * output interface to Qt
 */

#include <limits>
#include <cstdio>

#include <unistd.h>

#include "meta.h"
#include "array.h"
#include "output.h"

__MPT_NAMESPACE_BEGIN

// logger interfaces
int debug(const char *from, const char *fmt, ... )
{
    logger *log = logger::defaultInstance();
    if (!log) return 0;
    va_list va;
    if (fmt) va_start(va, fmt);
    int ret = log->log(from, log->Debug | log->LogFunction, fmt, va);
    if (fmt) va_end(va);
    return ret;
}
int warning(const char *from, const char *fmt, ... )
{
    logger *log = logger::defaultInstance();
    if (!log) return 0;
    va_list va;
    if (fmt) va_start(va, fmt);
    int ret = log->log(from, log->Warning | log->LogFunction, fmt, va);
    if (fmt) va_end(va);
    return ret;
}
int error(const char *from, const char *fmt, ... )
{
    logger *log = logger::defaultInstance();
    if (!log) return 0;
    va_list va;
    if (fmt) va_start(va, fmt);
    int ret = log->log(from, log->Error | log->LogFunction, fmt, va);
    if (fmt) va_end(va);
    return ret;
}
int critical(const char *from, const char *fmt, ... )
{
    logger *log = logger::defaultInstance();
    if (!log) return 0;
    va_list va;
    if (fmt) va_start(va, fmt);
    int ret = log->log(from, log->Critical | log->LogFunction, fmt, va);
    if (fmt) va_end(va);
    return ret;
}
int println(const char *fmt, ... )
{
    va_list va;
    if (fmt) va_start(va, fmt);
    logger *log;
    int ret = 0;
    if ((log = logger::defaultInstance())) {
        ret = log->log(0, log->Message | log->LogSelect, fmt, va);
    } else if (fmt) {
        ret = std::vprintf(fmt, va);
    }
    if (fmt) va_end(va);
    return ret;
}
logger *logger::defaultInstance()
{
    return mpt_log_default();
}
int logger::message(const char *from, int err, const char *fmt, ...)
{
    va_list va;
    if (fmt) va_start(va, fmt);
    int ret = log(from, err | LogFunction, fmt, va);
    if (fmt) va_end(va);
    return ret;
}

// logging store entry
logger::LogType LogStore::Entry::type() const
{
    Header *h;

    if (length() < sizeof(*h)) {
        return logger::Message;
    }
    h = (Header *) base();
    return (logger::LogType) h->type;
}
const char *LogStore::Entry::source() const
{
    Header *h;

    if (length() < (sizeof(*h) + 1)) {
        return 0;
    }
    h = (Header *) base();

    if (!h->from) {
        return 0;
    }
    return (const char *) (h + 1);
}
Slice<const char> LogStore::Entry::data(int part) const
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


int LogStore::Entry::set(const char *from, int type, const char *fmt, va_list arg)
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

LogStore::LogStore(metatype *next) : Reference<metatype>(next), _act(0), _flags(FlowNormal), _ignore(Debug), _level(0)
{ }
LogStore::~LogStore()
{ }
static int logMeta(const metatype *mt, const char *from, int type, const char *fmt, va_list arg)
{
    logger *l;
    if (!mt) {
        if (!(l = mpt_log_default())) {
            return 0;
        }
        return l->log(from, type, fmt, arg);
    }
    if ((l = mt->cast<logger>())) {
        return l->log(from, type, fmt, arg);
    }
    output *o;
    if ((o = mt->cast<output>())) {
        return mpt_output_vlog(o, from, type, fmt, arg);
    }
    return BadType;
}
int LogStore::log(const char *from, int type, const char *fmt, va_list arg)
{
    int save = 0, pass = 0, code = 0x7f & type;

    if (!code) {
        save |= _flags & SaveMessage;
        pass |= _flags & PassMessage;
    }
    else if (code >= _ignore) {
        save = (type & File) && (_flags & SaveLogAll);
        pass |= _flags & PassUnsaved;
    } else {
        save = 1;
        pass |= _flags & PassSaved;
    }
    if (type & File) {
        pass |= _flags & PassFile;
    }
    metatype *mt = pointer();
    // fast-track without argument list copy
    if (!save) {
        if (mt && pass) {
            return logMeta(mt, from, type, fmt, arg);
        }
        return 0;
    }
    if (mt && pass) {
        va_list tmp;
        // use copy to keep data for storage operation
        va_copy(tmp, arg);
        return logMeta(mt, from, type, fmt, arg);
        va_end(tmp);
    }
    Entry m;
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

const LogStore::Entry *LogStore::nextEntry()
{
    Entry *e;

    if ((e = _msg.get(_act))) {
        ++_act;
    }
    return e;
}

void LogStore::clearLog()
{
    _msg.resize(0);
    _act = 0;
    _level = 0;
}

bool LogStore::setIgnoreLevel(int val)
{
    if (val < 0) val = Debug;
    else if (val >= File) return false;
    _ignore = val;
    return true;
}
bool LogStore::setFlowFlags(int val)
{
    _flags = val;
    return true;
}

__MPT_NAMESPACE_END
