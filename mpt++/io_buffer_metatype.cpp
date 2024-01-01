/*
 * MPT C++ buffer metatype implementation
 */

#include <sys/uio.h>

#include "io.h"

__MPT_NAMESPACE_BEGIN

// initialize metatype wrapper
io::buffer::metatype::metatype(const array &arr) : buffer(arr)
{ }
// create metatype instance
io::buffer::metatype *io::buffer::metatype::create(const array *arr)
{
	class meta_buffer : public metatype
	{
	public:
		inline meta_buffer(const array &arr) : metatype(arr), _ref(1)
		{ }
		void unref() __MPT_OVERRIDE
		{
			if (_ref.lower()) {
				return;
			}
			delete this;
		}
		uintptr_t addref() __MPT_OVERRIDE
		{
			return _ref.raise();
		}
	private:
		refcount _ref;
	};
	return arr ? new meta_buffer(*arr) : new meta_buffer(array(0));
}
// convertable interface
int io::buffer::metatype::convert(type_t type, void *ptr)
{
	int me = type_properties<io::interface *>::id(true);
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
			span<const uint8_t> d = data();
			vec->iov_base = const_cast<uint8_t *>(d.begin());
			vec->iov_len = d.size();
		}
		return me;
	}
	if (type == 's') {
		span<const uint8_t> d = data();
		if (!memchr(d.begin(), 0, d.size())) {
			return BadValue;
		}
		if (ptr) *static_cast<const uint8_t **>(ptr) = d.begin();
		return me;
	}
	return BadType;
}
// metatype interface
io::buffer::metatype *io::buffer::metatype::clone() const
{
	if (_enc) {
		if (_state.scratch || _state._ctx) {
			return 0;
		}
	}
	io::buffer::metatype *b = io::buffer::metatype::create(&_d);
	b->_state = _state;
	return b;
}

__MPT_NAMESPACE_END
