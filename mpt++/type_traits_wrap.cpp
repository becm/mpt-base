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

extern int type_traits::add(const type_traits &traits)
{
	return mpt_type_generic_new(&traits);
}

__MPT_NAMESPACE_END
