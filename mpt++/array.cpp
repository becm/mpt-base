/*
 * MPT C++ buffer implementation
 */

#include <cerrno>

#include <sys/uio.h>

#include "message.h"
#include "convert.h"
#include "queue.h"
#include "meta.h"

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

// pointer array compact redirection
size_t compact(void **base, size_t len)
{ return mpt_array_compact(base, len); }

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
EncodingArray::EncodingArray(DataEncoder enc) : _enc(enc)
{ }
EncodingArray::~EncodingArray()
{ }

Slice<uint8_t> EncodingArray::data() const
{
    uint8_t *base = (uint8_t *) _d.base();
    size_t off = _d.length() - (_state.done - _state.scratch);

    return Slice<uint8_t>(base + off, _state.done);
}
bool EncodingArray::trim(size_t len)
{
    if (!len) {
        _state.done = 0;
        return true;
    }
    if (len > _state.done) {
        return false;
    }
    _state.done -= len;
    return true;
}
ssize_t EncodingArray::push(size_t len, const void *data)
{
    struct iovec vec;

    vec.iov_base = (void *) data;
    vec.iov_len  = len;

    return mpt_array_push(&_d, &_state, _enc, len ? &vec : 0);
}

bool EncodingArray::setData(const message *msg)
{
    if (!msg) {
        if (mpt_array_push(&_d, &_state, _enc, 0) < 0) {
            return false;
        }
        return true;
    }

    struct iovec *cont, vec;
    size_t clen, total;

    vec.iov_base = (void *) msg->base;
    vec.iov_len  = msg->used;

    cont = msg->cont;
    clen = msg->clen;
    total = 0;

    while (1) {
        if (!vec.iov_len) {
            if (!clen--) {
                break;
            }
            vec = *(cont++);
            continue;
        }
        ssize_t curr = mpt_array_push(&_d, &_state, _enc, &vec);

        if (curr < 0 || (size_t) curr > vec.iov_len) {
            if (total) {
                vec.iov_base = 0;
                vec.iov_len  = 1;
                (void) mpt_array_push(&_d, &_state, _enc, &vec);
            }
            return false;
        }
    }
    if (mpt_array_push(&_d, &_state, _enc, 0) < 0) {
        if (total) {
            vec.iov_base = 0;
            vec.iov_len  = 1;
            (void) mpt_array_push(&_d, &_state, _enc, &vec);
        }
        return false;
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
    size_t len = ::mpt::compact(begin(), length());
    _d.set(len * sizeof(void *));
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
    if (!val) {
        mpt_array_clone(&_d, 0);
        return 0;
    }
    const char *base;
    size_t len;
    if (!val->fmt) {
        if (!(base = (const char *) val->ptr)) {
            len = 0;
        } else {
            len = strlen(base);
        }
        if (!mpt_array_append(&_d, len, base)) {
            return BadOperation;
        }
        return len;
    }
    value tmp = *val;

    while (*tmp.fmt) {
        /* insert space element */
        if (tmp.fmt != val->fmt) {
            if (!mpt_array_append(&_d, 1, " ")) {
                return tmp.fmt - val->fmt;
            }
        }
        /* copy string value */
        if ((base = mpt_data_tostring(&tmp.ptr, *tmp.fmt, &len))) {
            if (!mpt_array_append(&_d, len, base)) {
                len = tmp.fmt - val->fmt;
                return len ? (int) len : BadOperation;
            }
            ++tmp.fmt;
            continue;
        }
        char buf[256];
        int ret;
        /* print binary data */
        if ((ret = mpt_data_print(buf, sizeof(buf), *tmp.fmt, tmp.ptr)) >= 0) {
            if (!mpt_array_append(&_d, ret, buf)) {
                len = tmp.fmt - val->fmt;
                return len ? (int) len : BadOperation;
            }
            if ((ret = mpt_valsize(*tmp.fmt++)) < 0) {
                return tmp.fmt - val->fmt;
            }
            tmp.ptr = ((uint8_t *) tmp.ptr) + ret;
            continue;
        }
    }
    len = tmp.fmt - val->fmt;
    return len ? (int) len : BadValue;
}
int Buffer::conv(int type, void *ptr)
{
    void **dest = (void **) ptr;

    if (type & MPT_ENUM(ValueConsume)) {
        return BadOperation;
    }
    if (!type) {
        static const char types[] = { 's', IODevice::Type, array::Type, 0 };
        if (dest) *dest = (void *) types;
        return IODevice::Type;
    }
    switch (type &= 0xff) {
    case metatype::Type: ptr = static_cast<metatype *>(this); break;
    case IODevice::Type: ptr = static_cast<IODevice *>(this); break;
    case array::Type:    ptr = static_cast<array *>(&_d); break;
    case 's': ptr = mpt_array_string(&_d); break;
    default: return BadType;
    }
    if (dest) *dest = ptr;
    return type;
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
        return 0;
    }
    if ((SIZE_MAX / nblk) < esze) {
        errno = ERANGE;
        return -2;
    }
    if (!esze) {
        return push(0, 0);
    }
    size_t left = nblk;
    while (nblk) {
        if (!push(esze, from)) {
            return esze - nblk;
            break;
        }
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
Slice<uint8_t> Buffer::peek(size_t see)
{
    Slice<uint8_t> d = EncodingArray::data();
    size_t len = d.length();
    return  Slice<uint8_t>(d.base(), see < len ? see : len);
}

__MPT_NAMESPACE_END
