/*!
 * MPT C++ stream and socket implementation
 */

#include <cstring>

#include <cstdio>
#include <cerrno>

#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "queue.h"
#include "array.h"
#include "event.h"
#include "message.h"
#include "convert.h"

#include "stream.h"

__MPT_NAMESPACE_BEGIN

int convert(const void **from, int ft, void *dest, int dt)
{ return mpt_data_convert(from, ft, dest, dt); }

float80 &float80::swapOrder(void)
{
    mpt_bswap_80(1, this);
    return *this;
}
float80 & float80::operator= (const long double &val)
{
    mpt_float80_encode(1, &val, this);
    return *this;
}
long double float80::value() const
{
    long double v;
    mpt_float80_decode(1, this, &v);
    return v;
}

void *output::typecast(int t)
{
    switch (t) {
      case metatype::Type: return static_cast<metatype *>(this);
      case output::Type: return this;
      default: return 0;
    }
}

int output::message(const char *from, int type, const char *fmt, ... )
{
    logger *log;
    const char mt[2] = { MessageOutput, (char) type };
    char buf[1024];
    int slen = 0;

    if ((log = cast<logger>())) {
        va_list ap;
        va_start(ap, fmt);
        slen = log->log(from, type, fmt, ap);
        va_end(ap);
        return slen;
    }
    if (push(sizeof(mt), &mt) < 0) {
        return -1;
    }
    if (from && (slen = push(strlen(from)+1, from)) < 0) {
        slen = 0;
    }
    slen += sizeof(mt);

    if (fmt) {
        va_list ap;
        int alen;
        va_start(ap, fmt);
        alen = vsnprintf(buf, sizeof(buf), fmt, ap);
        va_end(ap);
        if (alen < 0) {
            alen = snprintf(buf, sizeof(buf), "<%s: %s>", MPT_tr("invalid format string"), fmt);
        }
        if (alen >= (int) sizeof(buf)) {
            alen = sizeof(buf) - 1;
        }
        if (alen && (alen = push(alen, buf)) > 0) {
            slen += alen;
        }
    }
    push(0, 0);

    return slen;
}

// streaminfo access
streaminfo::~streaminfo()
{
    _mpt_stream_setfile(this, -1, -1);
}
bool streaminfo::setFlags(int fl)
{
    if (fl & ~0xf0) return false;
    _fd |= fl;
    return true;
}

// stream data operations
stream::stream()
{
    _enc.fcn = 0;
    _dec.fcn = 0;
}
stream::~stream()
{
    mpt_stream_close(this);
}
int stream::flags() const
{
    return mpt_stream_flags(&this->_info) & 0xff;
}
int stream::errors() const {
    return MPT_stream_errors(mpt_stream_flags(&this->_info));
}
void stream::setError(int err)
{
    return mpt_stream_seterror(&this->_info, err);
}
bool stream::endline(void)
{
    int nl = (mpt_stream_flags(&this->_info) & 0xc000) >> 14;
    
    switch (nl) {
      case NewlineMac:  return mpt_stream_write(this, 1, "\r",   1);
      case NewlineUnix: return mpt_stream_write(this, 1, "\n",   1);
      case NewlineNet:  return mpt_stream_write(this, 1, "\r\n", 2);
      default: return false;
    }
}
void stream::setNewline(int nl, int what)
{
    return mpt_stream_setnewline(&this->_info, nl, what);
}
bool stream::open(const char *fn, const char *mode)
{
    if (!fn) { mpt_stream_close(this); return true; }
    return mpt_stream_open(this, fn, mode) < 0 ? false : true;
}

// socket data operations
socket::~socket()
{ mpt_bind(this, 0, 0, 0); }

bool socket::bind(const char *addr, int listen)
{
    return (mpt_bind(this, addr, 0, listen) < 0) ? false : true;
}

bool socket::set(source &src)
{
    socket tmp;
    const char *dst;
    if (src.conv('s', &dst) < 0 || !dst || mpt_connect(&tmp, dst, 0) < 0) {
        return false;
    }
    if (_id) {
        (void) close(_id);
    }
    _id = tmp._id;
    return true;
}

// stream class
Stream::Stream(const streaminfo *from, uintptr_t ref) : _ref(ref), _inputFile(-1)
{
    if (!from) return;
    _mpt_stream_setfile(&_info, _mpt_stream_fread(from), _mpt_stream_fwrite(from));
    int flags = mpt_stream_flags(from);
    mpt_stream_setmode(this, flags & 0xff);
    setNewline(MPT_stream_newline_read(flags),  StreamRead);
    setNewline(MPT_stream_newline_write(flags), StreamWrite);
}

Stream::~Stream()
{
    mpt_command_clear(&_msg);
}

int Stream::unref()
{
    uintptr_t c = _ref;
    if (!c || (c = --_ref)) return c;
    delete this;
    return 0;
}

Stream *Stream::addref()
{
    intptr_t c = _ref;
    if (!c) return 0;
    if ((c = ++_ref) > 0) return this;  // limit to avoid race conditions
    --_ref; return 0;
}

int Stream::property(struct property *pr, source *src)
{
    if (!pr) {
        return src ? mpt_stream_setter(this, src) : Type;
    }

    int ret;
    intptr_t pos;
    const char *name;
    if (!(name = pr->name)) {
        if (src || ((pos = (intptr_t) pr->desc) < 0)) {
            errno = EINVAL;
            return -3;
        }
    }
    else if (!*name) {
        if ((ret = mpt_stream_setter(this, src)) < 0) return ret;
        pr->name = "stream";
        pr->desc = "generic data stream";
        pr->val.fmt = "";
        pr->val.ptr = static_cast<stream *>(this);
        return ret;
    }
    intptr_t id = 0;
    if (name ? strcasecmp(name, "idlen") : (pos == id++)) {
        if (!src) {
            ret = id;
        } else {
            uint8_t l;
            if ((ret = src->conv('C', &l)) < 0) return ret;
            if (l > sizeof(uintptr_t)) {
                errno = ERANGE; return -2;
            }
        }
        pr->name = "idlen";
        pr->desc = "message id length";
        pr->val.fmt = "C";
        pr->val.ptr = &_idlen;
        return ret;
    }
    errno = EINVAL;
    return -1;
}
void *Stream::typecast(int type)
{
    switch (type) {
    case IODevice::Type: return static_cast<IODevice *>(this);
    case input::Type:    return static_cast<input *>(this);
    case output::Type:   return static_cast<output *>(this);
    default: return 0;
    }
}

void Stream::close()
{
    mpt_stream_close(this);
    _inputFile = -1;
}
bool Stream::open(const char *dest, const char *type)
{
    if (_inputFile >= 0) return false;
    return mpt_stream_open(this, dest, type) < 0 ? false : true;
}
bool Stream::open(void *base, size_t len, int mode)
{
    if (_inputFile >= 0) {
        return false;
    }
    if (len && !base) {
        return false;
    }
    struct iovec data = { base, len };
    int ret;
    switch (mode) {
    case StreamRead:  ret = mpt_stream_memory(this, &data, 0); break;
    case StreamWrite: ret = mpt_stream_memory(this, 0, &data); break;
    default: return false;
    }
    if (ret < 0) {
        return false;
    }
    return true;
}

ssize_t Stream::write(size_t len, const void *data, size_t part)
{
    return mpt_stream_write(this, len, data, part);
}
ssize_t Stream::read(size_t len, void *data, size_t part)
{
    return mpt_stream_read(this, len, data, part);
}

int64_t Stream::pos()
{ return mpt_stream_seek(this, 0, SEEK_CUR); }

bool Stream::seek(int64_t pos)
{ return mpt_stream_seek(this, pos, SEEK_SET) >= 0 ? true : false; }


int Stream::next(int what)
{
    return mpt_stream_poll(this, what, 0);
}
int Stream::dispatch(EventHandler cmd, void *arg)
{
    if (!cmd) {
        return (mpt_queue_peek(&_rd, &_dec.info, _dec.fcn, 0) < 0) ? -1 : 0;
    }
    return mpt_stream_dispatch(this, _idlen, cmd, arg);
}
int Stream::_file()
{
    if (_inputFile >= 0) {
        return _inputFile;
    }
    if ((_inputFile = _mpt_stream_fread(&_info)) >= 0) {
        return _inputFile;
    }
    return _inputFile = _mpt_stream_fwrite(&_info);
}

ssize_t Stream::push(size_t len, const void *src)
{
    return mpt_stream_push(this, len, src);
}
int Stream::sync(int timeout)
{
    return mpt_stream_sync(this, _idlen, &_msg, timeout);
}

int Stream::await(int (*rctl)(void *, const struct message *), void *rpar)
{
    struct command *cmd;

    if (mpt_stream_flags(&_info) & StreamMesgAct) {
        errno = EAGAIN;
        return -2;
    }
    if (!(cmd = mpt_message_nextid(&_msg))) {
        return -1;
    }
    if (!rctl) {
        return 0;
    }
    cmd->cmd = (int (*)(void *, void *)) rctl;
    cmd->arg = rpar;
    return 1;
}

int Stream::log(const char *from, int type, const char *fmt, va_list va)
{
    char buf[1024];
    int len = 0;

    if (fmt) {
        if ((len = vsnprintf(buf, sizeof(buf)-1, fmt, va)) < 0) {
            *buf = 0;
            len  = 1;
        }
        else if (len >= (int) sizeof(buf)) {
            len = sizeof(buf);
            buf[len-4] = buf[len-3] = buf[len-2] = '.';
            buf[len-1] = 0;
        }
    }

    if (_enc.fcn) {
        uint8_t hdr[2] = { 0, (uint8_t) type };
        mpt_stream_push(this, 2, hdr);
    }
    if (from) {
        mpt_stream_push(this, strlen(from), from);
        mpt_stream_push(this, 2, ": ");
    }
    if (len) {
        mpt_stream_push(this, len, buf);
    }
    if (_enc.fcn) {
        mpt_stream_push(this, 0, 0);
    } else {
        mpt_stream_push(this, 1, "\n");
    }
    return 1;
}

int Stream::getchar()
{
    if (_rd.len) {
        uint8_t *base = (uint8_t *) _rd.base;
        if (_rd.off >= _rd.max) {
            _rd.off = 0;
        }
        --_rd.len;
        return base[_rd.off++];
    }
    return IODevice::getchar();
}

// socket class
Socket::Socket(struct socket *from, uintptr_t ref) : Metatype(ref)
{
    if (!from) return;
    *static_cast<socket *>(this) = *from;
    new (from) socket;
}
Socket::~Socket()
{ }

Socket *Socket::addref()
{ return Metatype::addref() ? this : 0; }

int Socket::property(struct property *pr, source *src)
{
    if (pr) {
        return -1;
    }
    if (!src) {
        return Type;
    }
    return (socket::set(*src)) ? 0 : -2;
}
void *Socket::typecast(int type)
{
    switch (type) {
    case metatype::Type: return static_cast<metatype *>(this);
    case socket::Type: return static_cast<metatype *>(this);
    default: return 0;
    }
}

Reference<Stream> Socket::accept()
{
    int sock;

    if ((sock = ::accept(_id, 0, 0)) < 0) {
        return 0;
    }
    streaminfo info;

    if (_mpt_stream_setfile(&info, sock, sock) < 0) {
        return 0;
    }
    info.setFlags(StreamBuffer);

    Stream *s = new Stream(&info);

    return s;
}

__MPT_NAMESPACE_END


