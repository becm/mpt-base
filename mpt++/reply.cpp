/*
 * MPT C++ event reply context handling
 */

#include <stdlib.h>

#include "array.h"

#include "event.h"

__MPT_NAMESPACE_BEGIN

/*!
 * \ingroup mptEvent
 * \brief stream context
 * 
 * Create new stream context entry.
 * 
 * \param ptr  reference to context base pointer
 * 
 * \return created stream context
 */
reply_context *reply_context::array::reserve(size_t len)
{
    return mpt_reply_reserve(&_d, len);
}
void reply_context::unref()
{
    if (used) {
        ptr = 0;
        return;
    }
    free(this);
}

__MPT_NAMESPACE_END
