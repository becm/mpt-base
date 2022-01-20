/*
 * MPT C++ queue I/O operations
 */

#include "queue.h"

#include "io.h"

__MPT_NAMESPACE_BEGIN

// queue with raw throughput
io::queue::queue(size_t len)
{
	if (len) mpt_queue_prepare(&_d, len);
}
io::queue::~queue()
{
	mpt_queue_resize(&_d, 0);
}
bool io::queue::prepare(size_t len = 1)
{
	return (!len || mpt_queue_prepare(&_d, len)) ? true : false;
}
bool io::queue::push(const void *data, size_t len = 1)
{
	mpt_queue_prepare(&_d, len);
	return mpt_qpush(&_d, len, data) >= 0;
}
bool io::queue::pop(void *data = 0, size_t len = 1)
{
	if (data) {
		return mpt_qpop(&_d, len, data);
	}
	return mpt_queue_crop(&_d, _d.len - len, len) >= 0;
}
bool io::queue::unshift(const void *data = 0, size_t len = 1)
{
	mpt_queue_prepare(&_d, len);
	return mpt_qunshift(&_d, len, data) >= 0;
}
bool io::queue::shift(void *data = 0, size_t len = 1)
{
	if (data) {
		return mpt_qshift(&_d, len, data);
	}
	return mpt_queue_crop(&_d, 0, len) >= 0;
}
// I/O device interface
ssize_t io::queue::write(size_t len, const void *d, size_t part)
{
	size_t done = 0;
	if (!part) {
		return prepare(len) ? len : -1;
	}
	if (!prepare(part*len)) {
		prepare(part);
	}
	while (done < len) {
		if (!mpt_qpush(&_d, part, d)) {
			return done;
		}
		++done;
		d = ((char *) d) + part;
	}
	return len;
}
ssize_t io::queue::read(size_t len, void *d, size_t part)
{
	size_t done = 0;
	while (done < len) {
		if (!mpt_qpop(&_d, part, d)) {
			return done;
		}
		++done;
		d = ((char *) d) + part;
	}
	return len;
}
span<const uint8_t> io::queue::peek(size_t len)
{
	size_t low = 0;
	void *base = mpt_queue_data(&_d, &low);
	
	if (!len) len = _d.len;
	
	if (len <= low) {
		len = low;
	}
	else if (!_d.fragmented()) {
		len = _d.len;
	}
	else {
		mpt_queue_align(&_d, 0);
		base = _d.base;
		len = _d.len;
	}
	return span<const uint8_t>(static_cast<uint8_t *>(base), len);
}

__MPT_NAMESPACE_END
