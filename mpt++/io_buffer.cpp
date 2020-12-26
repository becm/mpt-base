/*
 * MPT C++ buffer implementation
 */

#include <cerrno>

#include <sys/uio.h>

#include "message.h"

#include "array.h"

#include "io.h"

__MPT_NAMESPACE_BEGIN

// buffer instance
io::buffer::buffer(array const &a)
{
	_d = a;
	_state.done = _d.length();
}
io::buffer::~buffer()
{ }
// convertable interface
int io::buffer::convert(int type, void *ptr)
{
	int me = type_properties<io::interface *>::id();
	if (me < 0) {
		me = TypeMetaPtr;
	}
	if (!type) {
		static const uint8_t types[] = { TypeIteratorPtr, MPT_type_toVector('c'), 's', 0 };
		if (ptr) *static_cast<const uint8_t **>(ptr) = types;
		return me;
	}
	if (assign(static_cast<io::interface *>(this), type, ptr)) {
		return me;
	}
	if (assign(static_cast<metatype *>(this), type, ptr)) {
		return me;
	}
	if (assign(static_cast<iterator *>(this), type, ptr)) {
		return me;
	}
	if (type == MPT_type_toVector('c')) {
		struct iovec *vec;
		if ((vec = static_cast<struct iovec *>(ptr))) {
			span<uint8_t> d = data();
			vec->iov_base = d.begin();
			vec->iov_len = d.size();
		}
		return me;
	}
	if (type == 's') {
		span<uint8_t> d = data();
		if (!memchr(d.begin(), 0, d.size())) {
			return BadValue;
		}
		if (ptr) *static_cast<const uint8_t **>(ptr) = d.begin();
		return me;
	}
	return BadType;
}
// metatype interface
void io::buffer::unref()
{
	delete this;
}
io::buffer *io::buffer::clone() const
{
	if (_enc) {
		if (_state.scratch || _state._ctx) {
			return 0;
		}
	}
	io::buffer *b = new io::buffer(_d);
	b->_state = _state;
	b->_enc = _enc;
	return b;
}
// iterator interface
int io::buffer::get(int type, void *ptr)
{
	if (!type) {
		int me = type_properties<io::interface *>::id();
		mpt_slice_get(0, type, ptr);
		return me < 0 ? type_properties<array>::id() : me;
	}
	if (_state.scratch) {
		return message::InProgress;
	}
	size_t off = _d.length() - _state.done;
	slice s(_d);
	s.shift(off);
	if ((type = mpt_slice_get(&s, type, ptr)) <= 0) {
		return type;
	}
	int me = type_properties<io::interface *>::id();
	return me < 0 ? TypeMetaPtr : me;
}
int io::buffer::advance()
{
	if (!_state.done) {
		return MissingData;
	}
	span<uint8_t> d = data();
	uint8_t *end;
	if (!(end = (uint8_t *) memchr(d.begin(), 0, d.size()))) {
		return BadValue;
	}
	shift((end + 1) - d.begin());
	return _state.done ? 's' : 0;
}
int io::buffer::reset()
{
	_state.done = _d.length() - _state.scratch;
	return _state.done;
}

// I/O device interface
ssize_t io::buffer::read(size_t nblk, void *dest, size_t esze)
{
	span<uint8_t> d = data();
	if (!esze) {
		if ((size_t) d.size() < nblk) {
			return MissingData;
		}
		if (dest) memcpy(dest, d.begin(), nblk);
		return nblk;
	}
	size_t avail = d.size() / esze;
	
	if (nblk > avail) nblk = avail;
	
	if (!nblk) {
		return 0;
	}
	avail = nblk * esze;
	if (dest) memcpy(dest, d.begin(), avail);
	
	shift(avail);
	
	return nblk;
}
ssize_t io::buffer::write(size_t nblk, const void *from, size_t esze)
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
	return left - nblk;
}
int64_t io::buffer::pos()
{
	size_t len = _state.done + _state.scratch;
	return _d.length() - len;
}
bool io::buffer::seek(int64_t pos)
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
span<uint8_t> io::buffer::peek(size_t)
{
	return encode_array::data();
}

__MPT_NAMESPACE_END
