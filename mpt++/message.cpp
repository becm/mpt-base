/*
 * mpt C++ library
 *   message operations
 */

#include "message.h"
#include "event.h"

__MPT_NAMESPACE_BEGIN

// message processing
size_t message::read(size_t len, void *base)
{ return mpt_message_read(this, len, base); }
size_t message::length() const
{ return mpt_message_length(this); }

// message source interface
int MessageSource::reply(const message *)
{
    return -1;
}

__MPT_NAMESPACE_END
