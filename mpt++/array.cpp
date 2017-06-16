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
long compact(Slice<void *> s)
{
    return mpt_array_compact(s.base(), s.length());
}
long unused(Slice<void *> s)
{
    long u = 0;
    for (void **b = s.begin(), **e = s.end(); b < e; ++b) {
        if (!*b) ++u;
    }
    return u;
}
bool swap(Slice<void *> s, long p1, long p2)
{
    long len = s.length();
    if (p1 > len || p2 > len) {
        return false;
    }
    void *t, **b = s.begin();
    t = b[p1];
    b[p1] = b[p2];
    b[p2] = t;
    return true;
}

// buffer operations
void buffer::unref()
{
    if (!mpt_reference_lower(&_ref)) {
         free(this);
    }
}
uintptr_t buffer::addref()
{
    return mpt_reference_raise(&_ref);
}

buffer::buffer(size_t post) : _size(post), _used(0)
{ }

// array data implementation
array::Data *array::Data::create(size_t len)
{
    void *b;
    len += sizeof(buffer);
    len = MPT_align(len);
    if (!(b = malloc(len))) {
        return 0;
    }
    return new (b) Data(len - sizeof(Data));
}

bool array::Data::setLength(size_t len)
{
    if (len > _size) return 0;
    if (len > _used) {
        uint8_t *ptr = static_cast<uint8_t *>(data());
        memset(ptr + _used, 0, len - _used);
    }
    _used = len;
    return true;
}
int array::Data::content() const
{
    return 0;
}
array::Data *array::Data::detach(long len)
{
    if (len < 0) {
        len = _used;
    }
    Data *d;
    // create copy
    if (shared()) {
        if (!(d = create(len))) {
            return 0;
        }
        if (_used) {
            memcpy(static_cast<void *>(d + 1), static_cast<void *>(this + 1), _used);
            d->_used = _used;
        }
        _ref.lower();
        return d;
    }
    size_t total = len + sizeof(*d);
    total = MPT_align(total);
    // skip for big data shrink
    if ((size_t) len <= _size && (size_t) len < _used / 2 && total <= 0x100) {
        // reduce data to available size
        if ((size_t) len < _used) {
            _used = len;
        }
        return this;
    }
    // set new buffer size
    if (!(d = static_cast<Data *>(realloc(this, total)))) {
        return 0;
    }
    d->_size = len;
    if ((size_t) len < d->_used) {
        d->_used = len;
    }
    return d;
}

// array initialisation
array::array(size_t len) : _buf(0)
{
    if (len) _buf = array::Data::create(len);
}
array::array(const array &a) : _buf(0)
{ *this = a; }

// copy buffer reference
array &array::operator= (const array &a)
{
    _buf = a._buf;
    return *this;
}
// copy buffer reference
bool array::set(const Reference<buffer> &a)
{
    buffer *b;
    if ((b = a.pointer()) && b->content()) {
        return false;
    }
    _buf = reinterpret_cast<const Reference<array::Data> &>(a);
    return true;
}
// array size modifier
array &array::operator= (struct iovec const& vec)
{ set(vec.iov_len, vec.iov_base); return *this; }

array &array::operator+= (struct iovec const& vec)
{ append(vec.iov_len, vec.iov_base); return *this; }

void *array::set(size_t len, const void *base)
{
    Data *d = _buf.pointer();
    if (d && d->content()) {
        d = 0;
    }
    if (!d) {
        if (!(d = Data::create(len))) {
            return 0;
        }
    }
    else if (!(d = d->detach(len))) {
        return 0;
    } else {
        _buf.detach();
    }
    _buf.setPointer(d);
    d->setLength(len);
    void *ptr = d->data();
    if (base) {
        memcpy(ptr, base, len);
    } else {
        memset(ptr, 0, len);
    }
    return ptr;
}

int array::set(metatype &src)
{
    struct ::iovec vec;

    if (src.conv(TypeVector, &vec) >= 0) {
        return (set(vec.iov_len, vec.iov_base)) ? 0 : BadOperation;
    }
    if (src.conv(MPT_value_toVector('c'), &vec) >= 0) {
        return (set(vec.iov_len, vec.iov_base)) ? 0 : BadOperation;
    }
    char *data;
    int len;
    if ((len = src.conv('s', &data)) < 0) {
        return len;
    }
    if (!len) {
        return MissingData;
    }
    len = strlen(data);
    if (!set(len + 1)) {
        return BadOperation;
    }
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
    array a;
    while (*val.fmt) {
        /* insert space element */
        if (fmt != val.fmt) {
            if (!mpt_array_append(&a, 1, "\0")) {
                return BadOperation;
            }
        }
        /* copy string value */
        if ((base = mpt_data_tostring(&val.ptr, *val.fmt, &len))) {
            if (!mpt_array_append(&a, len, base)) {
                return BadOperation;
            }
            ++val.fmt;
            continue;
        }
        char buf[256];
        int ret;
        /* print number data */
        if ((ret = mpt_number_print(buf, sizeof(buf), valfmt(), *val.fmt, val.ptr)) >= 0) {
            if (!mpt_array_append(&a, ret, buf)) {
                return BadOperation;
            }
            if ((ret = mpt_valsize(*val.fmt)) < 0) {
                return BadType;
            }
            ++val.fmt;
            val.ptr = ((uint8_t *) val.ptr) + ret;
            continue;
        }
        break;
    }
    mpt_array_clone(this, &a);
    return val.fmt - fmt;
}
void *array::append(size_t len, const void *data)
{
    void *base = mpt_array_append(this, len);
    if (!base) return 0;
    return data ? memcpy(base, data, len) : base;
}
void *array::insert(size_t off, size_t len, const void *data)
{
    size_t used = 0, min;
    Data *d;
    /* compatibility check */
    if ((d = _buf.pointer())) {
        int type;
        if ((type = d->content()) || type != 'c') {
            d->unref();
            _buf.detach();
        }
    }
    if (!d) {
        if (!(d = Data::create(off + len))) {
            return 0;
        }
    }
    else {
        min = off > used ? off : used;
        if (!(d = d->detach(min + len))) {
            return 0;
        }
        _buf.detach();
    }
    _buf.setPointer(d);
    void *dest;
    if (!(dest = mpt_buffer_insert(d, off, len))) {
        return 0;
    }
    if (data) {
        memcpy(dest, data, len);
    } else {
        memset(dest, 0, len);
    }
    return dest;
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

// data segment from array
slice::slice(slice const& from) : array(from), _off(0), _len(0)
{
    _buf = from._buf;
    if (!_buf.pointer()) return;
    _len = from._len;
    _off = from._off;
}
slice::slice(array::Data *b) : _off(0), _len(0)
{
    _buf.setPointer(b);
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

__MPT_NAMESPACE_END
