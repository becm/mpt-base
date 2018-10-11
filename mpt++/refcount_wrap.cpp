/*
 * mpt C++ library
 *   reference operations
 */

#include "core.h"

__MPT_NAMESPACE_BEGIN

uintptr_t refcount::raise()
{
	return mpt_refcount_raise(this);
}
uintptr_t refcount::lower()
{
	return mpt_refcount_lower(this);
}

__MPT_NAMESPACE_END
