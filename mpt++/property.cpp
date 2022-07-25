/*
 * mpt C++ library
 *   type ID classification
 */

#include <cstring>

#include "object.h"

__MPT_NAMESPACE_BEGIN

bool property::set(const char *ptr)
{
	bool complete = false;
	if (ptr) {
		int len = strlen(ptr) + 1;
		if (sizeof(_buf) >= (sizeof(ptr) + len)) {
			ptr = static_cast<char *>(memcpy(_buf + sizeof(ptr), ptr, len));
			complete = true;
		}
	}
	*((const char **) _buf) = ptr;
	val.set('s', _buf);
	return complete;
}

int property::set(int type, const void *ptr)
{
	const type_traits *traits;;
	if (!(traits = type_traits::get(type))) {
		return BadType;
	}
	if (traits->init || traits->fini) {
		return BadArgument;
	}
	if (traits->size > sizeof(_buf)) {
		return MissingBuffer;
	}
	if (ptr) {
		ptr = memcpy(_buf, ptr, traits->size);
	}
	else {
		ptr = memset(_buf, 0, traits->size);
	}
	val.set(type, ptr);
	
	return 0;
}

__MPT_NAMESPACE_END
