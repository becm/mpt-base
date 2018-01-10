/*
 * mpt C++ library
 *   message operations
 */

#include "message.h"

__MPT_NAMESPACE_BEGIN

// message processing
size_t message::read(size_t len, void *base)
{ return mpt_message_read(this, len, base); }
size_t message::length() const
{ return mpt_message_length(this); }

bool msgdest::match(msgdest dst, int flg) const
{
    if (flg & MatchLayout
     && dst.lay != lay) {
        return false;
    }
    if (flg & MatchGraph
     && dst.grf != grf) {
        return false;
    }
    if (flg & MatchWorld
     && dst.wld != wld) {
        return false;
    }
    if (flg & MatchDimension
     && dst.dim != dim) {
        return false;
    }
    return true;
}

__MPT_NAMESPACE_END
