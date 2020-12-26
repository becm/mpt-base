/*
 * mpt C++ library
 *   type trait operations
 */

#include "types.h"

__MPT_NAMESPACE_BEGIN

int type_id(const struct type_traits *traits)
{
	return mpt_type_id(traits);
}
extern const struct type_traits *type_traits(int type)
{
	return mpt_type_traits(type);
}

__MPT_NAMESPACE_END
