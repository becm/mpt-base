/*
 * MPT C++ buffer implementation
 */

#include <cerrno>

#include <sys/uio.h>

#include "message.h"
#include "convert.h"
#include "types.h"
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

// pointer slice compact redirection
long compact(span<void *> s)
{
	return mpt_array_compact(s.begin(), s.size());
}
long unused(span<void *> s)
{
	long u = 0;
	for (void **b = s.begin(), **e = s.end(); b < e; ++b) {
		if (!*b) ++u;
	}
	return u;
}
bool swap(span<void *> s, long p1, long p2)
{
	long len = s.size();
	if (p1 > len || p2 > len) {
		return false;
	}
	void *t, **b = s.begin();
	t = b[p1];
	b[p1] = b[p2];
	b[p2] = t;
	return true;
}
// buffer storage implementation
buffer *buffer::create(size_t len, const struct type_traits *traits)
{
	buffer *b = _mpt_buffer_alloc(len);
	if (b) {
		b->_content_traits = traits;
	}
	return b;
}
buffer *buffer::create_unique(size_t len, const struct type_traits *traits)
{
	buffer *b = _mpt_buffer_alloc_unique(len);
	if (b) {
		b->_content_traits = traits;
	}
	return b;
}
buffer *buffer::detach(size_t)
{
	return 0;
}
void *buffer::insert(size_t pos, size_t len)
{
	return mpt_buffer_insert(this, pos, len);
}
bool buffer::move(buffer &from)
{
	if (this == &from) {
		return true;
	}
	if (_size < from._used) {
		return false;
	}
	if (!trim(_used)) {
		return false;
	}
	memcpy(static_cast<void *>(this + 1), static_cast<void *>((&from) + 1), from._used);
	_used = from._used;
	from._used = 0;
	return true;
}
bool buffer::copy(const buffer &from)
{
	if (&from == this) {
		return true;
	}
	if (mpt_buffer_set(this, from._content_traits, 0, &from + 1, from._used) < 0) {
		return false;
	}
	if (from._used < _used) {
		trim(_used - from._used);
	}
	return true;
}
    
bool buffer::skip(size_t len)
{
	if (_used < len) {
		return false;
	}
	size_t post = _used - len;
	uint8_t *base = static_cast<uint8_t *>(static_cast<void *>(this + 1));
	const struct type_traits *traits;
	if ((traits = content_traits())) {
		size_t size;
		if (!(size = traits->size)
		 || len % size) {
			return false;
		}
		void (*fini)(void *);
		if ((fini = traits->fini)) {
			for (size_t i = 0; i < len; i += size) {
				fini(base + i);
			}
		}
	}
	std::memmove(base, base + len, post);
	_used = post;
	return true;
}
bool buffer::trim(size_t len)
{
	if (_used < len) {
		return false;
	}
	size_t used = _used;
	len = used - len;
	const struct type_traits *traits = content_traits();
	if (traits) {
		size_t size;
		if (!(size = traits->size)
		 || used % size
		 || len % size) {
			return false;
		}
		void (*fini)(void *);
		if ((fini = traits->fini)) {
			uint8_t *base = static_cast<uint8_t *>(static_cast<void *>(this + 1)) + len;
			for (size_t i = len; i < used; i += size) {
				fini(base + i);
			}
		}
	}
	_used = len;
	return true;
}
void *buffer::append(size_t len)
{
	size_t used = _used;
	size_t avail = _size - used;
	if (len > avail) {
		return 0;
	}
	uint8_t *base = static_cast<uint8_t *>(static_cast<void *>(this + 1)) + used;
	const struct type_traits *traits;
	if ((traits = content_traits())) {
		size_t size;
		if (!(size = traits->size)
		 || used % size
		 || len % size) {
			return 0;
		}
	}
	_used += len;
	return base;
}
// simple array data implementation
bool array::content::set_length(size_t len)
{
	if (content_traits()) {
		return false;
	}
	if (len > _size) {
		return false;
	}
	if (len > _used) {
		uint8_t *ptr = static_cast<uint8_t *>(data());
		memset(ptr + _used, 0, len - _used);
	}
	_used = len;
	return true;
}

// array initialisation
array::array(size_t len) : _buf(0)
{
	if (len) {
		_buf.set_instance(static_cast<content *>(buffer::create(len)));
	}
}
array::array(const array &a) : _buf(0)
{
	*this = a;
}
// copy buffer reference
array &array::operator= (const array &a)
{
	_buf = a._buf;
	return *this;
}
// copy buffer reference
bool array::set(const reference<buffer> &a)
{
	buffer *b;
	if ((b = a.instance()) && b->content_traits()) {
		return false;
	}
	_buf = reinterpret_cast<const reference<content> &>(a);
	return true;
}
// array size modifier
array &array::operator= (struct iovec const& vec)
{
	set(vec.iov_len, vec.iov_base);
	return *this;
}

array &array::operator+= (struct iovec const& vec)
{
	append(vec.iov_len, vec.iov_base);
	return *this;
}
void *array::set(size_t len, const void *base)
{
	content *d;
	
	if ((d = _buf.instance())) {
		size_t used;
		/* incompatible target buffer */
		if (d->content_traits() || d->shared()) {
			d = 0;
		}
		else if (len <= (used = d->length())) {
			d->set_length(len);
		}
		/* reserve uninitialized data */
		else if (!d->append(len - used)) {
			d = 0;
		}
	}
	if (!d) {
		if (!(d = static_cast<content *>(buffer::create(len)))) {
			return 0;
		}
		/* reserve uninitialized data */
		if (!d->append(len)) {
			d->unref();
			return 0;
		}
		_buf.set_instance(d);
	}
	void *ptr = d->data();
	if (base) {
		memcpy(ptr, base, len);
	} else {
		memset(ptr, 0, len);
	}
	return ptr;
}

int array::set(convertable &src)
{
	struct ::iovec vec;
	
	if (src.convert(TypeVector, &vec) >= 0) {
		return (set(vec.iov_len, vec.iov_base)) ? 0 : BadOperation;
	}
	if (src.convert(MPT_type_toVector('c'), &vec) >= 0) {
		return (set(vec.iov_len, vec.iov_base)) ? 0 : BadOperation;
	}
	char *data;
	int len;
	if ((len = src.convert('s', &data)) < 0) {
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
	const uint8_t *fmt;
	const char *base;
	size_t len;
	if (!(fmt = val.fmt)) {
		base = (const char *) val.ptr;
		len = base ? strlen(base) + 1 : 0;
		return set(len, base) ? 0 : BadOperation;
	}
	array a;
	while (*val.fmt) {
		/* copy string value */
		if ((base = mpt_data_tostring(&val.ptr, *val.fmt, &len))) {
			if (!mpt_array_append(&a, len, base)) {
				return BadOperation;
			}
			/* insert space element */
			if (!mpt_array_append(&a, 1, "\0")) {
				return BadOperation;
			}
			++val.fmt;
			continue;
		}
		char buf[256];
		int ret;
		/* print number data */
		if ((ret = mpt_number_print(buf, sizeof(buf), value_format(), *val.fmt, val.ptr)) >= 0) {
			const MPT_STRUCT(type_traits) *traits;
			if (!(traits = type_traits::get(*val.fmt))) {
				return BadType;
			}
			if (!mpt_array_append(&a, ret, buf)) {
				return BadOperation;
			}
			/* insert space element */
			if (!mpt_array_append(&a, 1, "\0")) {
				return BadOperation;
			}
			++val.fmt;
			val.ptr = ((uint8_t *) val.ptr) + traits->size;
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
	void *dest = 0;
	content *d;
	
	/* compatibility check */
	if ((d = _buf.instance())
	 && !d->content_traits()
	 && !d->shared()) {
		dest = d->insert(off, len);
	}
	if (!dest) {
		size_t total = off + len;
		if (!(d = static_cast<content *>(buffer::create(total)))) {
			return 0;
		}
		if (!(dest = d->insert(off, len))) {
			d->unref();
			return 0;
		}
	}
	dest = static_cast<uint8_t *>(dest) + off;
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
{
	return mpt_array_string(this);
}
// data segment from array
slice::slice(slice const& from) : array(from), _off(0), _len(0)
{
	_buf = from._buf;
	if (!_buf.instance()) {
		return;
	}
	_len = from._len;
	_off = from._off;
}
slice::slice(array::content *b) : _off(0), _len(0)
{
	_buf.set_instance(b);
	_len = length();
}
bool slice::shift(ssize_t len)
{
	if (len < 0) {
		if (-len > (ssize_t) _off) {
			return false;
		}
	}
	else if (len > (ssize_t) _len) {
		return false;
	}
	_len -= len;
	_off += len;
	return true;
}
bool slice::trim(ssize_t len)
{
	if (len < 0) {
		if (length() < (_off + _len - len)) {
			return false;
		}
	}
	else if ((size_t) len > _len) {
		return false;
	}
	_len -= len;
	return true;
}
int slice::set(convertable &src)
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
encode_array::encode_array(data_encoder_t enc) : _enc(enc)
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
span<uint8_t> encode_array::data() const
{
	uint8_t *base = (uint8_t *) _d.base();
	size_t off = _d.length() - _state.done - _state.scratch;
	
	return span<uint8_t>(base + off, _state.done);
}
/*!
 * \ingroup mptArray
 * \brief remove finished data
 * 
 * Remove data from completed buffer segment
 * or move used segment to buffer front (len = 0).
 * 
 * \param len  length of data to remove
 */
bool encode_array::shift(size_t len)
{
	// move data segment to front
	if (!len) {
		size_t max, len = _state.done + _state.scratch;
		if ((max = _d.length() <= len)) {
			return false;
		}
		uint8_t *d = reinterpret_cast<uint8_t *>(_d.base());
		size_t shift = max - len;
		memcpy(d, d + shift, len);
		_d.set(len);
		return true;
	}
	// consume terminated data
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
