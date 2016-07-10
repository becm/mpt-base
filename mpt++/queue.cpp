/*
 * MPT C++ queue implementation
 */

#include <poll.h>
#include <sys/uio.h>

#include <cstring>

#include "message.h"

#include "queue.h"

__MPT_NAMESPACE_BEGIN

// generic IODevice operations
Slice<uint8_t> IODevice::peek(size_t)
{ return Slice<uint8_t>(0, 0); }
int64_t IODevice::pos()
{ return -1; }
bool IODevice::seek(int64_t )
{ return false; }
int IODevice::getchar()
{
    uint8_t letter;
    ssize_t rv;
    if ((rv = read(1, &letter, sizeof(letter))) < (int) sizeof(letter))
        return rv < 0 ? rv : -1;
    return letter;
}

ssize_t decode_queue::receive()
{
    return mpt_queue_recv(this);
}
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

DecodingQueue::DecodingQueue(DataDecoder dec) : decode_queue(dec), _mlen(-1)
{ }
DecodingQueue::~DecodingQueue()
{
    mpt_queue_resize(this, 0);
}

bool DecodingQueue::currentMessage(message &msg, struct iovec *cont)
{
    if (_mlen < 0) {
        return false;
    }
    mpt_message_get(this, _state.done, _mlen, &msg, cont);

    return true;
}
bool DecodingQueue::pendingMessage()
{
    if (_mlen >= 0) {
        return true;
    }
    if ((_mlen = mpt_queue_recv(this)) < 0) {
        return false;
    }
    if (_state.done) {
        mpt_queue_crop(this, 0, _state.done);
        _state.done = 0;
    }
    return true;
}
bool DecodingQueue::advance()
{
    if (_mlen < 0) {
        return false;
    }
    mpt_queue_crop(this, 0, _mlen + _state.done);
    _state.scratch -= _mlen;
    _state.done = 0;

    _mlen = mpt_queue_recv(this);

    return true;
}

// queue metatype operations
Queue::Queue(size_t len)
{
    if (len) Queue::prepare(len);
}
Queue::~Queue()
{ mpt_queue_resize(&_d, 0); }

// queue metatype interface
void Queue::unref()
{
    delete this;
}

bool Queue::prepare(size_t len = 1)
{ return (!len || mpt_queue_prepare(&_d, len)) ? true : false; }

bool Queue::push(const void *data, size_t len = 1)
{
    mpt_queue_prepare(&_d, len);
    return mpt_qpush(&_d, len, data) >= 0;
}
bool Queue::pop(void *data = 0, size_t len = 1)
{
    if (data) return mpt_qpop(&_d, len, data);
    return mpt_queue_crop(&_d, _d.len - len, len) >= 0;
}
bool Queue::unshift(const void *data = 0, size_t len = 1)
{
    mpt_queue_prepare(&_d, len);
    return mpt_qunshift(&_d, len, data) >= 0;
}
bool Queue::shift(void *data = 0, size_t len = 1)
{
    if (data) return mpt_qshift(&_d, len, data);
    return mpt_queue_crop(&_d, 0, len) >= 0;
}

Slice<uint8_t> Queue::peek(size_t len)
{
    size_t low = 0;
    void *base = mpt_queue_data(&_d, &low);

    if (!len) len = _d.len;

    if (len <= low) {
        len = low;
    }
    else if (!_d.fragmented()) {
        len = _d.len;
    }
    else {
        mpt_queue_align(&_d, 0);
        base = _d.base;
        len = _d.len;
    }
    return Slice<uint8_t>(static_cast<uint8_t *>(base), len);
}

ssize_t Queue::write(size_t len, const void *d, size_t part)
{
    size_t done = 0;
    if (!part) return prepare(len) ? len : -1;
    if (!prepare(part*len)) prepare(part);
    while (done < len) {
        if (!mpt_qpush(&_d, part, d)) return done;
        ++done;
        d = ((char *) d) + part;
    }
    return len;
}
ssize_t Queue::read(size_t len, void *d, size_t part)
{
    size_t done = 0;
    while (done < len) {
        if (!mpt_qpop(&_d, part, d)) return done;
        ++done;
        d = ((char *) d) + part;
    }
    return len;
}

__MPT_NAMESPACE_END
