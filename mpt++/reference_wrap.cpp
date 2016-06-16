/*
 * mpt C++ library
 *   reference operations
 */

#include "core.h"

__MPT_NAMESPACE_BEGIN

uintptr_t reference::raise()
{
    return mpt_reference_raise(this);
}
uintptr_t reference::lower()
{
    return mpt_reference_lower(this);
}

__MPT_NAMESPACE_END
