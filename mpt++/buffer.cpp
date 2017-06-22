/*
 * MPT C++ buffer implementation
 */

#include <cerrno>

#include <sys/uio.h>

#include "message.h"
#include "convert.h"

#include "../mptio/stream.h"

#include "array.h"

__MPT_NAMESPACE_BEGIN

// buffer metatype
Buffer::Buffer(array const &a)
{
    _d = a;
}
Buffer::~Buffer()
{ }
void Buffer::unref()
{
    delete this;
}
Buffer *Buffer::clone() const
{
    if (_enc) {
        if (_state.scratch || _state._ctx) {
	    return 0;
	}
    }
    Buffer *b = new Buffer(_d);
    b->_state = _state;
    b->_enc = _enc;
    return b;
}
int Buffer::conv(int type, void *ptr) const
{
    if (!type) {
        static const char types[] = { metatype::Type, MPT_value_toVector('c'), 's', 0 };
        if (ptr) *((const char **) ptr) = types;
        return Type;
    }
    if (type == metatype::Type) {
        if (ptr) *((void **) ptr) = static_cast<metatype *>(const_cast<Buffer *>(this));
        return Type;
    }
    if (type == MPT_value_toVector('c')) {
        struct iovec *vec;
        if ((vec = static_cast<struct iovec *>(ptr))) {
            Slice<uint8_t> d = data();
            vec->iov_base = d.begin();
            vec->iov_len = d.length();
        }
        return Type;
    }
    return BadType;
}
int Buffer::get(int type, void *ptr)
{
    if (!type) {
        value *val;
        if ((val = static_cast<value *>(ptr))) {
            static const char fmt[] = { buffer::Type, 0 };
            val->fmt = fmt;
            val->ptr = &_d;
        }
        return Type;
    }
    if (_state.scratch) return MessageInProgress;
    size_t off = _d.length() - _state.done;
    slice s(_d);
    s.shift(off);
    type = mpt_slice_get(&s, type, ptr);
    s.shift(s.data().length() - _state.done);
    return Type;
}

// iterator extensions for buffer
int Buffer::advance()
{
    return MissingData;
}
int Buffer::reset()
{
    return BadOperation;
}

// I/O interface implementation
ssize_t Buffer::read(size_t nblk, void *dest, size_t esze)
{
    Slice<uint8_t> d = data();
    if (!esze) {
        if ((size_t) d.length() < nblk) return -2;
        if (dest) memcpy(dest, d.base(), nblk);
        return nblk;
    }
    size_t avail = d.length() / esze;

    if (nblk > avail) nblk = avail;

    if (!nblk) return 0;

    avail = nblk * esze;
    if (dest) memcpy(dest, d.base(), avail);

    shift(avail);

    return nblk;
}
ssize_t Buffer::write(size_t nblk, const void *from, size_t esze)
{
    if (!nblk) {
        return push(0, 0);
    }
    if ((SIZE_MAX / nblk) < esze) {
        errno = ERANGE;
        return -2;
    }
    if (!esze) {
        return 0;
    }
    size_t left = nblk;
    while (nblk) {
        bool wait = _state.scratch;
        if (!push(esze, from)) {
            break;
        }
        if (!wait && !_enc) push(0, 0);
        from = ((uint8_t *) from) + esze;
        --nblk;
    }
    return nblk - left;

}
int64_t Buffer::pos()
{
    size_t len = _state.done + _state.scratch;
    return _d.length() - len;
}
bool Buffer::seek(int64_t pos)
{
    if (pos >= 0) {
        if ((size_t) pos > _state.done) {
            return false;
        }
        _state.done -= pos;
        return true;
    }
    ssize_t off = _state.done + _state.scratch;

    pos = -pos;
    if (off < pos) {
        return false;
    }
    _state.done += pos;

    return true;
}
Slice<uint8_t> Buffer::peek(size_t)
{
    return encode_array::data();
}

__MPT_NAMESPACE_END
