/*!
 * MPT C++ stream input operations
 */

#include "../mptio/stream.h"

#include "io.h"

__MPT_NAMESPACE_BEGIN

// input instance for stream
io::stream::input::input(const streaminfo *info) : io::stream(info)
{ }
// input instance for stream
io::stream::input *io::stream::input::create(const streaminfo *info)
{
	class stream_input : public input
	{
	public:
		inline stream_input(const streaminfo *info) : input(info), _ref(1)
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
	return new stream_input(info);
}
// convertable interface
int io::stream::input::convert(int type, void *ptr)
{
	int me = type_properties<io::interface *>::id(true);
	
	if (me < 0) {
		const named_traits *traits = mpt_input_type_traits();
		me = traits ? traits->type : type_properties<output *>::id(true);
	}
	else if (assign(static_cast<io::interface *>(this), type, ptr)) {
		return TypeUnixSocket;
	}
	if (!type) {
		static const uint8_t fmt[] = { TypeOutputPtr, TypeUnixSocket, 0 };
		if (ptr) *static_cast<const uint8_t **>(ptr) = fmt;
		return me;
	}
	if (type == TypeUnixSocket) {
		if (ptr) *reinterpret_cast<int *>(ptr) = _inputFile;
		return me;
	}
	if (assign(static_cast<metatype *>(this), type, ptr)) {
		return TypeOutputPtr;
	}
	if (assign(static_cast<output *>(this), type, ptr)) {
		return me;
	}
	return BadType;
}
// metatype interface
input *io::stream::input::clone() const
{
	// undefined state for side-effect structures
	return 0;
}
// input interface
int io::stream::input::next(int what)
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
int io::stream::input::dispatch(event_handler_t cmd, void *arg)
{
	const class dispatch sd(*this, cmd, arg);
	return stream::dispatch(sd);
}

__MPT_NAMESPACE_END
