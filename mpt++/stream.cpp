/*!
 * MPT C++ stream implementation
 */

#include <inttypes.h>

#include <cstring>

#include <cstdio>
#include <cerrno>

#include <poll.h>

#include <sys/uio.h>

#include "queue.h"
#include "array.h"
#include "message.h"

#include "output.h"
#include "meta.h"

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

// stream class
Stream::Stream(const streaminfo *from) : _inputFile(-1)
{
    if (!from) return;
    _srm = new stream;
    _mpt_stream_setfile(&_srm->_info, _mpt_stream_fread(from), _mpt_stream_fwrite(from));
    int flags = mpt_stream_flags(from);
    mpt_stream_setmode(_srm, flags & 0xff);
    _srm->setNewline(MPT_stream_newline_read(flags),  StreamRead);
    _srm->setNewline(MPT_stream_newline_write(flags), StreamWrite);
}
Stream::~Stream()
{ }

// metatype interface
void Stream::unref()
{
    if (_srm) delete _srm;
    delete this;
}
int Stream::assign(const value *val)
{
    if (val) {
        return setProperty(0, 0);
    } else {
        return mpt_object_pset(this, 0, val);
    }
}
int Stream::conv(int type, void *ptr)
{
    static const char fmt[] = { output::Type, input::Type, IODevice::Type, 0 };
    const void *addr = 0;
    switch (type) {
      case 0: addr = fmt; type = Type;
      case metatype::Type: addr = static_cast<metatype *>(this); break;
      case output::Type: addr = static_cast<output *>(this); break;
      case input::Type: addr = static_cast<input *>(this); break;
      case IODevice::Type: addr = static_cast<IODevice *>(this); break;
      default: return BadType;
    }
    if (ptr) *reinterpret_cast<const void **>(ptr) = addr;
    return type;
}
// object interface
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
            return BadValue;
        }
    }
    else {
        // get stream interface types
        if (!*name) {
            static const char fmt[] = { output::Type, input::Type, IODevice::Type, 0 };
            pr->name = "stream";
            pr->desc = "interfaces to stream data";
            pr->val.fmt = fmt;
            pr->val.ptr = 0;
            return _srm ? mpt_stream_flags(&_srm->_info) : 0;
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
        return _srm ? mpt_stream_setter(_srm, src) : BadOperation;
    }
    if (strcasecmp(pr, "idlen")) {
        if (_inputFile >= 0) {
            return BadOperation;
        }
        int ret;
        uint8_t l;

        if (!src) {
            ret = l = 0;
        } else {
            if ((ret = src->conv('y', &l)) < 0) return ret;
            if (l > sizeof(uintptr_t)) {
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
    if (!_srm) _srm = new stream;
    return mpt_stream_open(_srm, dest, type) < 0 ? false : true;
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
    if (!_srm) _srm = new stream;
    switch (mode) {
    case StreamRead:  ret = mpt_stream_memory(_srm, &data, 0); break;
    case StreamWrite: ret = mpt_stream_memory(_srm, 0, &data); break;
    default: return false;
    }
    if (ret < 0) {
        return false;
    }
    return true;
}
// IODevice interface
ssize_t Stream::write(size_t len, const void *data, size_t part)
{
    if (!_srm) {
        return BadArgument;
    }
    return mpt_stream_write(_srm, len, data, part);
}
ssize_t Stream::read(size_t len, void *data, size_t part)
{
    if (!_srm) {
        return BadArgument;
    }
    return mpt_stream_read(_srm, len, data, part);
}
int64_t Stream::pos()
{
    if (!_srm) {
        return BadArgument;
    }
    return mpt_stream_seek(_srm, 0, SEEK_CUR);
}
bool Stream::seek(int64_t pos)
{
    if (!_srm) {
        return BadArgument;
    }
    return mpt_stream_seek(_srm, pos, SEEK_SET) >= 0 ? true : false;
}
// input interface
int Stream::next(int what)
{
    if (!_srm) {
        return BadArgument;
    }
    int ret = mpt_stream_poll(_srm, what, 0);
    if (ret < 0) {
        _inputFile = -1;
    }
    return ret;
}
static int streamReply(void *ptr, const message *msg)
{
    static const char _func[] = "mpt::stream::reply";
    reply_context *rc = reinterpret_cast<reply_context *>(ptr);
    stream *srm = reinterpret_cast<stream *>(rc->ptr);
    uint64_t id = 0;

    if (!rc->used) {
        critical(_func, "%s: %s", MPT_tr("unable to reply"), id, MPT_tr("reply context not registered"));
        return rc->BadContext;
    }
    mpt_message_buf2id(rc->val, rc->len, &id);
    if (!(srm = reinterpret_cast<stream *>(rc->ptr))) {
        error(_func, "%s (id = " PRIx64 ")", MPT_tr("reply target destroyed"), id);
        if (!--rc->used) {
            free(rc);
        }
        return rc->BadDescriptor;
    }
    if (!rc->len) {
        error(_func, "%s (id = " PRIx64 ")", MPT_tr("no id size specified"), id);
        return rc->BadState;
    }
    if (srm->flags() & StreamMesgAct) {
        warning(_func, "%s (id = " PRIx64 ")", MPT_tr("message in progress"), id);
        return MessageInProgress;
    }
    rc->val[0] |= 0x80;
    if (mpt_stream_push(srm, rc->len, rc->val) < 0) {
        error(_func, "%s (id = " PRIx64 ")", MPT_tr("unable to set reply id"), id);
        return rc->BadPush;
    }
    rc->len = 0;
    --rc->used;
    if (mpt_stream_append(srm, msg) < 0) {
        if (msg && mpt_stream_push(srm, 0, 0) < 0) {
            critical(_func, "%s: %s", MPT_tr("bad reply operation"), id, MPT_tr("not able to terminate reply"));
        } else {
            warning(_func, "%s: %s", MPT_tr("bad reply operation"), id, MPT_tr("not able append message"));
        }
    }
    return 0;
}
class Stream::Dispatch
{
public:
    Dispatch(Stream &s, EventHandler c, void *a) : srm(s), cmd(c), arg(a)
    { }
    int dispatch(const struct message *msg)
    {
        static const char _func[] = "mpt::Stream::dispatch";
        struct event ev;
        uint8_t idlen;

        if (!(idlen = srm._idlen)) {
            return cmd(arg, &ev);
        }
        uint8_t id[__UINT8_MAX__];
        int ret;

        struct message tmp = *msg;
        if (tmp.read(idlen, id) < idlen) {
            error(_func, "%s", MPT_tr("message id incomplete"));
        }
        if (id[0] & 0x80) {
            command *ans;
            uint64_t rid;
            id[0] &= 0x7f;
            if ((ret = mpt_message_buf2id(id, idlen, &rid)) < 0 || ret > (int) sizeof(ans->id)) {
                error(_func, "%s", MPT_tr("bad reply id"));
                return BadValue;
            }
            if ((ans = srm._wait.get(rid))) {
                return ans->cmd(ans->arg, &tmp);
            }
            error(_func, "%s (id = %08"PRIx64")", MPT_tr("unknown reply id"), rid);
            return BadValue;
        }
        reply_context *rc = 0;
        for (uint8_t i = 0; i < idlen; ++i) {
            if (!id[i]) {
                continue;
            }
            if (!(rc = srm._ctx.reserve(srm._idlen))) {
                error(_func, "%s", MPT_tr("unable to create reply context"));
                return BadOperation;
            }
            rc->ptr = srm._srm;
            rc->len = idlen;
            memcpy(rc->val, id, idlen);
            ev.reply.set = streamReply;
            ev.reply.context = rc;
            break;
        }
        ret = cmd(arg, &ev);
        if (rc && ret < 0 && rc->len) {
            struct msgtype mt(MessageAnswer, ret);
            struct message msg(&mt, sizeof(mt));
            streamReply(rc, &msg);
        }
        return ret;
    }
protected:
    Stream &srm;
    EventHandler cmd;
    void *arg;
};
static int streamDispatch(void *ptr, const struct message *msg)
{
    Stream::Dispatch *sd = reinterpret_cast<Stream::Dispatch *>(ptr);
    return sd->dispatch(msg);
}
int Stream::dispatch(EventHandler cmd, void *arg)
{
    if (!_srm) {
        return BadArgument;
    }
    if (_srm->_mlen < 0
        && (_srm->_mlen = mpt_queue_recv(&_srm->_rd)) < 0) {
        if (_mpt_stream_fread(&_srm->_info) < 0) {
            return BadArgument;
        }
        return 0;
    }
    class Dispatch sd(*this, cmd, arg);
    return mpt_stream_dispatch(_srm, streamDispatch, &sd);
}
int Stream::_file()
{
    if (_inputFile >= 0) {
        return _inputFile;
    }
    if (!_srm) {
        return BadArgument;
    }
    if ((_inputFile = _mpt_stream_fread(&_srm->_info)) >= 0) {
        return _inputFile;
    }
    return _inputFile = _mpt_stream_fwrite(&_srm->_info);
}

ssize_t Stream::push(size_t len, const void *src)
{
    if (!_srm) {
        return BadArgument;
    }
    ssize_t curr;
    if (_idlen && !(_srm->flags() & MessageInProgress)) {
        uint8_t id[__UINT8_MAX__];
        mpt_message_id2buf(_cid, id, _idlen);
        if (mpt_stream_push(_srm, _idlen, id) < _idlen) {
            push(1, 0);
        }
    }
    if ((curr = mpt_stream_push(_srm, len, src)) < 0) {
        return curr;
    }
    if (!len) {
        _cid = 0;
    }
    else if (!src && _cid) {
        command *c;
        if ((c = _wait.get(_cid))) {
            c->cmd(c->arg, 0);
        }
    }
    return curr;
}
int Stream::sync(int timeout)
{
    if (!_srm || !_idlen) {
        return BadArgument;
    }
    return mpt_stream_sync(_srm, _idlen, &_wait, timeout);
}

int Stream::await(int (*rctl)(void *, const struct message *), void *rpar)
{
    if (!_idlen) {
        return BadArgument;
    }
    struct command *cmd;

    if (mpt_stream_flags(&_srm->_info) & StreamMesgAct) {
        return MessageInProgress;
    }
    if (!rctl) {
        return 0;
    }
    if (!(cmd = _wait.next(_idlen))) {
        return BadOperation;
    }
    _cid = cmd->id;
    cmd->cmd = (int (*)(void *, void *)) rctl;
    cmd->arg = rpar;
    return 1;
}

int Stream::log(const char *from, int type, const char *fmt, va_list va)
{
    if (!_srm) {
        return BadArgument;
    }
    // check no active message is composed
    if (mpt_stream_flags(&_srm->_info) & StreamMesgAct) {
        return MPT_ERROR(MessageInProgress);
    }
    // message header for encoded data
    if (_srm->_wd.encoded()) {
        static const msgtype mt(MessageOutput, (uint8_t) type);
        uint8_t len = _idlen;

        while (len) {
            static const uint8_t id[8] = { 0 };
            if (len <= sizeof(id)) {
                mpt_stream_push(_srm, len, id);
                break;
            }
            len -= sizeof(id);
            mpt_stream_push(_srm, sizeof(id), id);
        }
        mpt_stream_push(_srm, 2, &mt);
    }
    if (from) {
        mpt_stream_push(_srm, strlen(from), from);
        if (_srm->_wd.encoded()) {
            mpt_stream_push(_srm, 1, "");
        } else {
            mpt_stream_push(_srm, 2, ": ");
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
        mpt_stream_push(_srm, len, buf);
    }
    // terminate and flush message */
    mpt_stream_push(_srm, 0, 0);
    mpt_stream_flush(_srm);
    return 1;
}

int Stream::getchar()
{
    if (!_srm || _srm->_rd.encoded()) {
        return BadArgument;
    }
    if (_srm->_rd.len) {
        uint8_t *base = (uint8_t *) _srm->_rd.base;
        if (_srm->_rd.off >= _srm->_rd.max) {
            _srm->_rd.off = 0;
        }
        --_srm->_rd.len;
        return base[_srm->_rd.off++];
    }
    return IODevice::getchar();
}

__MPT_NAMESPACE_END


