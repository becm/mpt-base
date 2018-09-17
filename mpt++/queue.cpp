/*
 * MPT C++ queue implementation
 */

#include "message.h"

#include "queue.h"

__MPT_NAMESPACE_BEGIN

// coded queue target
ssize_t encode_queue::push(size_t len, const void *data)
{
    return mpt_queue_push(this, len, data);
}
bool encode_queue::trim(size_t take)
{
    if (_state.done > len) {
        return false;
    }
    if (take > _state.done) {
        return false;
    }
    mpt_queue_crop(this, 0, take);
    _state.done -= take;
    return true;
}
// coded queue source
bool decode_queue::advance()
{
    if (mpt_queue_recv(this) < 0) {
        return false;
    }
    mpt_queue_shift(this);
    return true;
    
}
bool decode_queue::current_message(message &msg, struct iovec *cont) const
{
    if (_state.data.msg < 0) {
        return false;
    }
    if (mpt_message_get(this, _state.data.pos, _state.data.msg, &msg, cont) < 0) {
        return false;
    }
    return true;
}

__MPT_NAMESPACE_END
