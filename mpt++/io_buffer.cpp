/*
 * MPT C++ buffer implementation
 */

#include <cerrno>

#include <sys/uio.h>

#include "io.h"

__MPT_NAMESPACE_BEGIN

// buffer instance
io::buffer::buffer(array const &a)
{
	_d = a;
	// access content to avoid type traits check array base/length
	const array::content *c;
	if ((c = _d.data())) {
		_state.done = c->length();
	}
}
io::buffer::~buffer()
{ }
// iterator interface
const struct value *io::buffer::value()
{
	if (_state.scratch || !_state.done) {
		return 0;
	}
	span<const uint8_t> d = data();
	const char *begin = (const char *) d.begin();
	const char *end;
	if (!(end = (char *) memchr(begin, 0, d.size()))) {
		return 0;
	}
	// save current delimited data
	_record = span<const char>(begin, end + 1 - begin);
	_value = _record;
	return &_value;
}
int io::buffer::advance()
{
	if (!_state.done) {
		return MissingData;
	}
	span<const uint8_t> d = data();
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
	span<const uint8_t> d = data();
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
span<const uint8_t> io::buffer::peek(size_t)
{
	return encode_array::data();
}

__MPT_NAMESPACE_END
