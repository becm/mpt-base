/*
 * MPT C++ queue implementation
 */

#include "io.h"

__MPT_NAMESPACE_BEGIN

const named_traits *io::interface::get_traits()
{
	static const named_traits *traits = 0;
	if (!traits && !(traits = mpt_type_interface_add("mpt.io"))) {
		traits = mpt_type_interface_add(0);
	}
	return traits;
}

template <> int type_properties<io::interface *>::id(bool obtain)
{
	static const named_traits *traits = 0;
	if (traits) {
		return traits->type;
	}
	if (obtain && !(traits = io::interface::get_traits())) {
		return BadOperation;
	}
	return traits ? traits->type : static_cast<int>(BadType);
}

template <> const struct type_traits *type_properties<io::interface *>::traits()
{
	static const struct type_traits *traits = 0;
	if (!traits) {
		const named_traits *nt = io::interface::get_traits();
		if (nt) {
			traits = &nt->traits;
		}
		else {
			static const struct type_traits fallback(sizeof(io::interface *));
			traits = &fallback;
		}
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
