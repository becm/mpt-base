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

// template specifications for C-implementations
template<> void copy<double, double>(int pts, const double *src, int lds, double *dest, int ldd)
{ mpt_copy64(pts, src, lds, dest, ldd); }
template<> void copy<uint64_t, uint64_t>(int pts, const uint64_t *src, int lds, uint64_t *dest, int ldd)
{ mpt_copy64(pts, src, lds, dest, ldd); }
template<> void copy<int64_t, int64_t>(int pts, const int64_t *src, int lds, int64_t *dest, int ldd)
{ mpt_copy64(pts, src, lds, dest, ldd); }

template<> void copy<float, float>(int pts, const float *src, int lds, float *dest, int ldd)
{ mpt_copy32(pts, src, lds, dest, ldd); }
template<> void copy<uint32_t, uint32_t>(int pts, const uint32_t *src, int lds, uint32_t *dest, int ldd)
{ mpt_copy32(pts, src, lds, dest, ldd); }
template<> void copy<int32_t, int32_t>(int pts, const int32_t *src, int lds, int32_t *dest, int ldd)
{ mpt_copy32(pts, src, lds, dest, ldd); }

template<> void copy<double, float>(int pts, const double *src, int lds, float *dest, int ldd)
{ mpt_copy_df(pts, src, lds, dest, ldd); }
template<> void copy<float, double>(int pts, const float *src, int lds, double *dest, int ldd)
{ mpt_copy_fd(pts, src, lds, dest, ldd); }

// pointer slice compact redirection
void compact(Slice<void *> &s)
{
    size_t len = mpt_array_compact(s.base(), s.length());
    s.trim(s.length() - len);
}

// buffer operations
void buffer::unref()
{
    if (shared--) return;
    if (resize) resize(this, 0);
}
uintptr_t buffer::addref()
{
    if (++shared) return shared;
    --shared; return 0;
}

buffer::buffer() : resize(_mpt_buffer_realloc), shared(0), size(0), used(0)
{ }
buffer::~buffer()
{ if (resize) resize(this, 0); }

// array initialisation
array::array(const array &a) : _buf(0)
{ *this = a; }

// copy buffer reference
array &array::operator= (const array &a)
{
    if (_buf) _buf->unref();
    _buf = (a._buf && a._buf->addref()) ? a._buf : 0;
    return *this;
}
// copy buffer reference
array &array::operator= (const Reference<buffer> &a)
{
    if (_buf) _buf->unref();
    Reference<buffer> t = a;
    _buf = t.detach();
    return *this;
}
// create buffer reference
const Reference<buffer> &array::ref() const
{
    return *((Reference<buffer> *) this);
}
// array size modifier
array &array::operator= (struct iovec const& vec)
{ set(vec.iov_len, vec.iov_base); return *this; }

array &array::operator+= (struct iovec const& vec)
{ append(vec.iov_len, vec.iov_base); return *this; }

void *array::set(size_t len, const void *base)
{
    if (!_buf) {
        if (!(_buf = _mpt_buffer_realloc(0, len))) return 0;
    }
    else if (len > _buf->size) {
        buffer *b;
        if (!_buf->resize || !(b = _buf->resize(_buf, len))) return 0;
        _buf = b;
    }
    if (base) {
        memcpy(_buf+1, base, len);
    }
    if (_buf->used < len) {
        memset(((uint8_t *)(_buf+1))+_buf->used, 0, len-_buf->used);
    }
    _buf->used = len;
    return _buf+1;
}

int array::set(metatype &src)
{
    struct ::iovec vec;

    if (src.conv(TypeVecBase, &vec) >= 0) {
        return (set(vec.iov_len, vec.iov_base)) ? vec.iov_len : -1;
    }
    char *data;
    int len;
    if ((len = src.conv('s', &data)) < 0) {
        if ((len = src.conv(0, &data)) < 0) return -2;
        if (len && !data) return -1;
        if (!set(len, data)) return -1;
        return len;
    }
    if (!data) {
        if (len) {
            return -1;
        }
        set(0);
        return 0;
    }
    else if (!len) {
        len = strlen(data);
    }
    if (!set(len+1)) return -1;
    data = (char *) memcpy(base(), data, len);
    data[len] = 0;

    return len;
}
int array::set(value val)
{
    const char *fmt, *base;
    size_t len;
    if (!(fmt = val.fmt)) {
        if (!(base = (const char *) val.ptr)) {
            len = 0;
        } else {
            len = strlen(base);
        }
        if (!mpt_array_append(this, len, base)) {
            return BadOperation;
        }
        return len;
    }
    while (*val.fmt) {
        /* insert space element */
        if (fmt != val.fmt) {
            if (!mpt_array_append(this, 1, " ")) {
                return val.fmt - fmt;
            }
        }
        /* copy string value */
        if ((base = mpt_data_tostring(&val.ptr, *val.fmt, &len))) {
            if (!mpt_array_append(this, len, base)) {
                len = val.fmt - fmt;
                return len ? (int) len : BadOperation;
            }
            ++val.fmt;
            continue;
        }
        char buf[256];
        int ret;
        /* print number data */
        if ((ret = mpt_number_print(buf, sizeof(buf), valfmt(), *val.fmt, val.ptr)) >= 0) {
            if (!mpt_array_append(this, ret, buf)) {
                len = val.fmt - fmt;
                return len ? (int) len : BadOperation;
            }
            if ((ret = mpt_valsize(*val.fmt++)) < 0) {
                return val.fmt - fmt;
            }
            val.ptr = ((uint8_t *) val.ptr) + ret;
            continue;
        }
    }
    len = val.fmt - fmt;
    return len ? (int) len : BadValue;
}
void *array::append(size_t len, const void *data)
{
    void *base = mpt_array_append(this, len);
    if (!base) return 0;
    return data ? memcpy(base, data, len) : base;
}
void *array::prepend(size_t len, size_t off)
{
    return mpt_array_insert(this, off, len);
}
int array::printf(const char *fmt, ... )
{
    va_list ap;
    va_start(ap, fmt);
    int ret = mpt_vprintf(this, fmt, ap);
    va_end(ap);
    return ret;
}
char *array::string()
{ return mpt_array_string(this); }

// typed information for array
bool typed_array::setType(int t)
{
    int esize;
    if ((esize = mpt_valsize(t)) <= 0) {
        errno = EINVAL;
        return false;
    }
    _format = t;
    _esize = esize;
    return true;
}
void typed_array::setModified(bool set)
{
    if (set) _flags |= ValueChange;
    else _flags &= ~ValueChange;
}
size_t maxsize(Slice<const typed_array> sl, int type)
{
    const typed_array *arr = sl.base();
    size_t len = 0;
    for (size_t i = 0, max = sl.length(); i < max; ++i) {
        if (!arr->elementSize()) {
            continue;
        }
        if (type >= 0 && type != arr->type()) {
            continue;
        }
        size_t curr = arr->elements();
        if (curr > len) len = curr;
    }
    return len;
}

// data segment from array
slice::slice(slice const& from) : array(from), _off(0), _len(0)
{
    if (!_buf) return;
    _len = from._len;
    _off = from._off;
}
slice::slice(buffer *b) : _off(0), _len(0)
{
    if (!(_buf = b)) return;
    _len = length();
}
bool slice::shift(ssize_t len)
{
    if (len < 0) {
       if (-len > (ssize_t) _off) return false;
    }
    else if (len > (ssize_t) _len) return false;
    _len -= len;
    _off += len;
    return true;
}
bool slice::trim(ssize_t len)
{
    if (len < 0) {
        if (length() < (_off + _len - len)) return false;
    }
    else if ((size_t) len > _len) return false;
    _len -= len;
    return true;
}
int slice::set(metatype &src)
{
    int len = array::set(src);
    if (len < 0) {
        return len;
    }
    _off = 0;
    _len = length();
    return len;
}
ssize_t slice::write(size_t nblk, const void *addr, size_t esze)
{
    return mpt_slice_write(this, nblk, addr, esze);
}


// encoding array
encode_array::encode_array(DataEncoder enc) : _enc(enc)
{ }
encode_array::~encode_array()
{
    if (_enc) _enc(&_state, 0, 0);
}

bool encode_array::prepare(size_t len)
{
    if (_enc) {
        return false;
    }
    size_t old = _d.length();
    if (!_d.set(old + len)) {
        return false;
    }
    _d.set(old);
    return true;
}
Slice<uint8_t> encode_array::data() const
{
    uint8_t *base = (uint8_t *) _d.base();
    size_t off = _d.length() - _state.done - _state.scratch;

    return Slice<uint8_t>(base + off, _state.done);
}
bool encode_array::trim(size_t len)
{
    if (!len) {
        size_t len, max = _state.done + _state.scratch;
        if ((len = _d.length() <= max)) {
            return false;
        }
        uint8_t *d = reinterpret_cast<uint8_t *>(_d.base());
        size_t shift = max - len;
        memcpy(d, d+shift, len);
        _d.set(len);
        return true;
    }
    if (len > _state.done) {
        return false;
    }
    _state.done -= len;
    return true;
}
ssize_t encode_array::push(size_t len, const void *data)
{
    return mpt_array_push(this, len, data);
}

bool encode_array::push(const struct message &msg)
{
    message tmp = msg;

    while (1) {
        if (!tmp.used) {
            if (!tmp.clen--) {
                break;
            }
            ++tmp.cont;
            continue;
        }
        ssize_t curr = mpt_array_push(this, tmp.used, tmp.base);

        if (curr < 0 || (size_t) curr > tmp.used) {
            return false;
        }
    }
    return true;
}

// basic pointer array
size_t PointerArray::unused() const
{
    void **pos = begin();
    size_t elem = 0, len = length();
    for (size_t i = 0; i < len; ++i) {
        if (!pos[i]) ++elem;
    }
    return elem;
}
void PointerArray::compact()
{
    Slice<void *> s(begin(), length());
    ::mpt::compact(s);
    _d.set(s.length() * sizeof(void *));
}
bool PointerArray::swap(size_t p1, size_t p2) const
{
    size_t len = length();
    if (p1 > len || p2 > len) return false;
    void *t, **b = begin();
    t = b[p1];
    b[p1] = b[p2];
    b[p2] = t;
    return true;
}
bool PointerArray::move(size_t p1, size_t p2) const
{
    size_t len = length();
    if (p1 >= len || p2 >= len) return false;
    ::mpt::move(begin(), p1, p2);
    return true;
}
bool PointerArray::insert(size_t pos, void *ref)
{
    void **b;
    if (_d.shared() || !(b = (void **) _d.prepend(sizeof(*b), pos*sizeof(*b)))) return false;
    *b = ref;
    return true;
}
ssize_t PointerArray::offset(const void *ref) const
{
    void **pos = begin();
    size_t len = length();
    for (size_t i = 0; i < len; ++i) {
         if (ref == pos[i]) return i;
    }
    return -1;
}

// buffer metatype
Buffer::Buffer(const Reference<buffer> &a)
{
    _d = a;
}
Buffer::~Buffer()
{ }
void Buffer::unref()
{
    delete this;
}
int Buffer::assign(const value *val)
{
    if (_enc) {
        return BadOperation;
    }
    if (!val) {
        _state.scratch = 0;
        _state.done = _d.length();
        return 0;
    }
    return _d.set(*val);
}
int Buffer::conv(int type, void *ptr)
{
    if (!type) {
        static const char types[] = { metatype::Type, MPT_value_toVector('c'), 's', 0 };
        if (ptr) *((const char **) ptr) = types;
        return Type;
    }
    if (type == metatype::Type) {
        if (ptr) *((void **) ptr) = static_cast<metatype *>(this);
        return type;
    }
    int curr = type & 0xff;
    if (curr == MPT_value_toVector('c')) {
        Slice<uint8_t> d = data();
        if (ptr) memcpy(ptr, &d, sizeof(d));
        if ((type & ValueConsume) && d.length()) trim(d.length());
        return type & (0xff | ValueConsume);
    }
    if (_state.scratch) return MessageInProgress;
    size_t off = _d.length() - _state.done;
    slice s(_d);
    s.shift(off);
    curr = mpt_slice_conv(&s, type, ptr);
    s.shift(s.data().length() - _state.done);
    return curr;
}
ssize_t Buffer::read(size_t nblk, void *dest, size_t esze)
{
    Slice<uint8_t> d = data();
    if (!esze) {
        if (d.length() < nblk) return -2;
        if (dest) memcpy(dest, d.base(), nblk);
        return nblk;
    }
    size_t avail = d.length() / esze;

    if (nblk > avail) nblk = avail;

    if (!nblk) return 0;

    avail = nblk * esze;
    if (dest) memcpy(dest, d.base(), avail);

    trim(avail);

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
