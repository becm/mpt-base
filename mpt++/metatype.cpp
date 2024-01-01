/*
 * MPT C++ metatype operations
 */

#include "meta.h"

__MPT_NAMESPACE_BEGIN

// default conversion
int metatype::convert(type_t type, void *ptr)
{
	void **dest = (void **) ptr;
	
	/* identify as metatype */
	if (!type) {
		static const uint8_t types[] = { TypeConvertablePtr, 0 };
		if (dest) *dest = (void *) types;
		return 0;
	}
	/* support (up-)cast to metatype pointer */
	if (assign(this, type, ptr)) {
		return type;
	}
	return BadType;
}

const named_traits *metatype::pointer_traits(void)
{
	static const named_traits *traits = 0;
	if (!traits) {
		traits = mpt_metatype_traits(TypeMetaPtr);
	}
	return traits;
}

const type_traits *metatype::reference_traits(void)
{
	static const type_traits *traits = 0;
	if (!traits) {
		traits = mpt_meta_reference_traits();
	}
	return traits;
}

__MPT_NAMESPACE_END
