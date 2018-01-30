/*!
 * MPT C++ stream implementation
 */

#include <inttypes.h>

#include <cstdio>
#include <limits>

#include <sys/uio.h>

#include "stream.h"

__MPT_NAMESPACE_BEGIN

template <> int typeinfo<Stream *>::id()
{
    static int id;
    if (!id && (id = mpt_valtype_meta_new("stream")) < 0) {
        id = mpt_valtype_meta_new(0);
    }
    return id;
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
    const char *endl = mpt_newline_string(nl);
    if (!endl) {
        return false;
    }
    return mpt_stream_write(this, 1, endl, strlen(endl));
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
Stream::Stream(const streaminfo *from) : _srm(0), _cid(0), _inputFile(-1), _idlen(0)
{
    if (!from) return;
    _srm = new stream;
    _mpt_stream_setfile(&_srm->_info, _mpt_stream_fread(from), _mpt_stream_fwrite(from));
    int flags = mpt_stream_flags(from);
    mpt_stream_setmode(_srm, flags & 0xff);
    _srm->setNewline(MPT_stream_newline_read(flags),  _srm->Read);
    _srm->setNewline(MPT_stream_newline_write(flags), _srm->Write);
}
Stream::~Stream()
{ }

// reference interface
void Stream::unref()
{
    if (_srm) delete _srm;
    delete this;
}
// metatype interface
int Stream::conv(int type, void *ptr) const
{
    int me = mpt_input_typeid();
    
    if (me < 0) {
        me = output::Type;
    }
    if (!type) {
        static const uint8_t fmt[] = { output::Type, socket::Type, 0 };
        if (ptr) *static_cast<const uint8_t **>(ptr) = fmt;
        return me;
    }
    if (type == socket::Type) {
        if (ptr) *reinterpret_cast<int *>(ptr) = _inputFile;
        return me;
    }
    if (type == metatype::Type) {
        if (ptr) *static_cast<const metatype **>(ptr) = this;
        return output::Type;
    }
    if (type == output::Type) {
        if (ptr) *static_cast<const output **>(ptr) = this;
        return me;
    }
    if (type == me) {
        if (ptr) *static_cast<const input **>(ptr) = this;
        return socket::Type;
    }
    return BadType;
}
// object interface
int Stream::property(struct property *pr) const
{
    int me = mpt_input_typeid();
    
    if (me < 0) {
        me = output::Type;
    }
    if (!pr) {
        return me;
    }
    const char *name;
    intptr_t pos;

    if (!(name = pr->name)) {
        if (((pos = (intptr_t) pr->desc) < 0)) {
            return BadValue;
        }
    }
    else {
        // get stream interface types
        if (!*name) {
            static const uint8_t fmt[] = { output::Type, input::Type, 0 };
            pr->name = "stream";
            pr->desc = "interfaces to stream data";
            pr->val.fmt = fmt;
            pr->val.ptr = 0;
            return _srm ? mpt_stream_flags(&_srm->_info) : 0;
        }
    }
    intptr_t id = 0;
    if (name ? !strcasecmp(name, "idlen") : (pos == id++)) {
        static const uint8_t fmt[] = "y";
        pr->name = "idlen";
        pr->desc = "message id length";
        pr->val.fmt = fmt;
        pr->val.ptr = &_idlen;
        return id;
    }
    return BadArgument;
}
int Stream::setProperty(const char *pr, const metatype *src)
{
    if (!pr) {
        if (!_srm) {
            return BadOperation;
        }
        int ret = mpt_stream_setter(_srm, src);
        if (ret >= 0) {
            _ctx.setPointer(0);
        }
        return ret;
    }
    if (strcasecmp(pr, "idlen")) {
        if (_inputFile >= 0) {
            return BadOperation;
        }
        uint8_t l;

        if (!src) {
            l = 0;
        } else {
            int ret = src->conv('y', &l);
            if (ret < 0) return ret;
            if (l > std::numeric_limits<__decltype(_idlen)>::max() / 2 + 1) {
                return BadValue;
            }
        }
        _idlen = l;
        return 0;
    }
    return BadArgument;
}
bool Stream::open(const char *dest, const char *type)
{
    if (_inputFile >= 0) return false;
    if (!_srm) _srm = new stream;
    if (mpt_stream_open(_srm, dest, type) < 0) return false;
    _inputFile = _mpt_stream_fread(&_srm->_info);
    return true;
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
      case stream::Read:  ret = mpt_stream_memory(_srm, &data, 0); break;
      case stream::Write: ret = mpt_stream_memory(_srm, 0, &data); break;
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
    if (!_srm || mpt_stream_seek(_srm, pos, SEEK_SET) < 0) {
        return false;
    }
    return true;
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

        if (!msg || !(idlen = srm._idlen)) {
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
                int ret = ans->cmd(ans->arg, &tmp);
                ans->cmd = 0;
                return ret;
            }
            error(_func, "%s (id = %08" PRIx64 ")", MPT_tr("unknown reply id"), rid);
            return BadValue;
        }
        metatype *ctx = 0;
        reply_data *rd = 0;
        reply_context *rc = 0;
        for (uint8_t i = 0; i < idlen; ++i) {
            if (!id[i]) {
                continue;
            }
            if (!(ctx = srm._ctx.pointer())) {
                warning(_func, "%s", MPT_tr("no reply context"));
                break;
            }
            if (!(rd = ctx->cast<reply_data>())) {
                break;
            }
            if (!rd->setData(idlen, id)) {
                error(_func, "%s", MPT_tr("reply context unusable"));
                return BadOperation;
            }
            rc = ctx->cast<reply_context>();
            break;
        }
        ev.reply = rc;
        ret = cmd(arg, &ev);
        if (rc && rd && rd->active()) {
            struct msgtype mt(msgtype::Answer, ret);
            struct message msg(&mt, sizeof(mt));
            rc->reply(&msg);
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

ssize_t Stream::push(size_t len, const void *src)
{
    if (!_srm) {
        return BadArgument;
    }
    ssize_t curr;
    if (_idlen && !(_srm->flags() & stream::MesgActive)) {
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

    if (mpt_stream_flags(&_srm->_info) & _srm->MesgActive) {
        return message::InProgress;
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
