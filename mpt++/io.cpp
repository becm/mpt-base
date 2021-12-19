/*
 * MPT C++ queue implementation
 */

#include "io.h"

__MPT_NAMESPACE_BEGIN

template <> int type_properties<io::interface *>::id(bool obtain)
{
	static int _valtype = 0;
	int type;
	/* already a registerd type */
	if ((type = _valtype) > 0) {
		return type;
	}
	if (!obtain) {
		return BadType;
	}
	/* register named of fallback type */
	if ((type = mpt_type_interface_new("mpt.io")) < 0) {
		type = mpt_type_interface_new(0);
	}
	return _valtype = type;
}

template <> const struct type_traits *type_properties<io::interface *>::traits()
{
	static const struct type_traits *traits = 0;
	if (!traits && !(traits = type_traits::get(id(true)))) {
		static const struct type_traits fallback(sizeof(io::interface *));
		traits = &fallback;
	}
	return traits;
}

// generic I/O operations
span<uint8_t> io::interface::peek(size_t)
{
	return span<uint8_t>(0, 0);
}
int64_t io::interface::pos()
{
	return -1;
}
bool io::interface::seek(int64_t )
{
	return false;
}
int io::interface::getchar()
{
	uint8_t letter;
	ssize_t rv;
	if ((rv = read(1, &letter, sizeof(letter))) < (int) sizeof(letter)) {
		return rv < 0 ? rv : -1;
	}
	return letter;
}

__MPT_NAMESPACE_END
