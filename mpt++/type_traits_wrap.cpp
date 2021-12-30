/*
 * mpt C++ library
 *   type trait operations
 */

#include "types.h"

__MPT_NAMESPACE_BEGIN

extern const struct type_traits *type_traits::get(int type)
{
	return mpt_type_traits(type);
}

extern const named_traits *type_traits::get(const char *name, int len)
{
	return mpt_named_traits(name, len);
}

extern int type_traits::add(const type_traits &traits)
{
	return mpt_type_add(&traits);
}

extern int type_traits::add_basic(size_t size)
{
	return mpt_type_basic_add(size);
}

extern const named_traits *type_traits::add_metatype(const char *name)
{
	return mpt_type_metatype_add(name);
}

extern const named_traits *type_traits::add_interface(const char *name)
{
	return mpt_type_interface_add(name);
}

__MPT_NAMESPACE_END
