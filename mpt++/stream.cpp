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
#include "message.h"

#include "stream.h"

__MPT_NAMESPACE_BEGIN

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
stream::stream() : _mlen(0)
{ }
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
bool stream::endline()
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

bool socket::set(metatype &src)
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
bool socket::set(const value *val)
{
    if (!val) {
        mpt_bind(this, 0, 0, 0);
        return 0;
    }
    socket tmp;
    if (!val->fmt) {
        const char *dst = (const char *) val->ptr;
        if (!dst || mpt_connect(&tmp, dst, 0) < 0) {
            return false;
        }
        if (_id) {
            (void) close(_id);
        }
        _id = tmp._id;
        return true;
    }
    return false;
}

// stream class
Stream::Stream(const streaminfo *from) : _inputFile(-1)
{
    _conv.out = this;
    _conv.in  = this;
    _conv.log = this;
    _conv.dev = this;
    
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

// object interface
void Stream::unref()
{
    delete this;
}
int Stream::property(struct property *pr) const
{
    if (!pr) {
        return Type;
    }
    const char *name;
    intptr_t pos;

    if (!(name = pr->name)) {
        if (((pos = (intptr_t) pr->desc) < 0)) {
            errno = EINVAL;
            return -3;
        }
    }
    else {
        // get stream interface types
        if (!*name) {
            static const char fmt[] = { output::Type, input::Type, logger::Type, IODevice::Type, 0 };
            pr->name = "stream";
            pr->desc = "interfaces to stream data";
            pr->val.fmt = fmt;
            pr->val.ptr = &_conv;
            return mpt_stream_flags(&_info);
        }
    }
    intptr_t id = 0;
    if (name ? !strcasecmp(name, "idlen") : (pos == id++)) {
        pr->name = "idlen";
        pr->desc = "message id length";
        pr->val.fmt = "y";
        pr->val.ptr = &_idlen;
        return id;
    }
    return BadArgument;
}

int Stream::setProperty(const char *pr, metatype *src)
{
    if (!pr) {
        return mpt_stream_setter(this, src);
    }
    if (strcasecmp(pr, "idlen")) {
        if (_inputFile >= 0) {
            errno = EALREADY;
            return BadOperation;
        }
        int ret;
        uint8_t l;

        if (!src) {
            ret = l = 0;
        } else {
            if ((ret = src->conv('y', &l)) < 0) return ret;
            if (l > sizeof(uintptr_t)) {
                errno = ERANGE;
                return BadValue;
            }
        }
        _idlen = l;
        return ret;
    }
    return BadArgument;
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
    context *ctx = 0;
    
    if (_mlen < 0
        && (_mlen = mpt_queue_recv(&_rd)) < 0) {
        if (_mpt_stream_fread(&_info) < 0) {
            return -2;
        }
        return 0;
    }
    if (!cmd) {
        return 1;
    }
    if (_rd.encoded()) {
        if (!(ctx = _ctx.reserve(_idlen))) {
            return BadOperation;
        }
        ctx->len = _idlen;
        ctx->used = 1;
    }
    return mpt_stream_dispatch(this, ctx, cmd, arg);
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
    // check no active message is composed
    if (mpt_stream_flags(&_info) & StreamMesgAct) {
        return MPT_ERROR(MissingData);
    }
    // message header for encoded data
    if (_wd.encoded()) {
        uint8_t hdr[2] = { MessageOutput, (uint8_t) type };
        mpt_stream_push(this, 2, hdr);
    }
    if (from) {
        mpt_stream_push(this, strlen(from), from);
        if (_wd.encoded()) {
            mpt_stream_push(this, 1, "");
        } else {
            mpt_stream_push(this, 2, ": ");
        }
    }
    if (fmt) {
        char buf[1024];
        int len = 0;
        if ((len = vsnprintf(buf, sizeof(buf)-1, fmt, va)) < 0) {
            *buf = 0;
            len  = 1;
        }
        else if (len >= (int) sizeof(buf)) {
            len = sizeof(buf);
            buf[len-4] = buf[len-3] = buf[len-2] = '.';
            buf[len-1] = 0;
        }
        mpt_stream_push(this, len, buf);
    }
    // append message newline
    if (_wd.encoded()) {
        mpt_stream_push(this, 1, "\n");
    }
    // terminate and flush message */
    mpt_stream_push(this, 0, 0);
    mpt_stream_flush(this);
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
Socket::Socket(struct socket *from)
{
    if (!from) return;
    *static_cast<socket *>(this) = *from;
    new (from) socket;
}
Socket::~Socket()
{ }

void Socket::unref()
{
    delete this;
}
int Socket::assign(const value *val)
{
    return (socket::set(val)) ? (val ? 1 : 0) : BadOperation;
}
int Socket::conv(int type, void *ptr)
{
    void **dest = (void **) ptr;

    if (type & ValueConsume) {
        return BadOperation;
    }
    if (!type) {
        static const char types[] = { socket::Type, metatype::Type, 0 };
        if (dest) *dest = (void *) types;
        return socket::Type;
    }
    switch (type &= 0xff) {
    case metatype::Type: ptr = static_cast<metatype *>(this); break;
    case socket::Type: ptr = static_cast<metatype *>(this); break;
    default: return BadType;
    }
    if (dest) *dest = ptr;
    return type;
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


