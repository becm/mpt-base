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
int64_t IODevice::pos(void)
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

DecodingQueue::DecodingQueue(DataDecoder dec) : _dec(dec), _mlen(-1)
{ }
DecodingQueue::~DecodingQueue()
{
    mpt_queue_resize(&_d, 0);
    if (_dec) _dec(&_state, 0, 0);
}

bool DecodingQueue::currentMessage(message &msg, struct iovec *cont)
{
    if (!pendingMessage()) {
        return false;
    }
    mpt_message_get(&_d, _state.done, _mlen, &msg, cont);

    return true;
}
bool DecodingQueue::pendingMessage()
{
    if (_mlen >= 0) {
        return true;
    }
    if ((_mlen = mpt_queue_recv(&_d, &_state, _dec)) < 0) {
        return false;
    }
    if (_state.done) {
        mpt_queue_crop(&_d, 0, _state.done);
        _state.done = 0;
    }
    return true;
}
bool DecodingQueue::advance()
{
    if (_mlen < 0) {
        return false;
    }
    mpt_queue_crop(&_d, 0, _mlen + _state.done);
    _state.scratch -= _mlen;
    _state.done = 0;

    _mlen = mpt_queue_recv(&_d, &_state, _dec);

    return true;
}

// queue metatype operations
Queue::Queue(size_t len, uintptr_t ref) : _ref(ref)
{
    if (len) Queue::prepare(len);
}
Queue::~Queue()
{ mpt_queue_resize(&_d, 0); }

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

Slice<uint8_t> Queue::data()
{
    uint8_t *base = (uint8_t *) _d.base;
    if (_d.fragmented()) mpt_queue_align(&_d, 0);
    else base += _d.off;
    return Slice<uint8_t>(base, _d.len);
}
// queue metatype interface
int Queue::unref()
{
    if (!_ref) return -1;
    if (--_ref) return _ref;
    delete this;
    return 0;
}
Queue *Queue::addref()
{
    if (!_ref) return 0;
    if (++_ref) return this;
    --_ref;
    return 0;
}
int Queue::assign(const value *val)
{
    if (!val) {
        return mpt_queue_prepare(&_d, 0) ? 1 : 0;
    }
    if (!val->fmt) {
        const char *d = (const char *) val->ptr;
        return d ? write(1, d, strlen(d)) : 0;
    }
    return BadType;
}
void *Queue::typecast(int type)
{
    switch (type) {
    case metatype::Type: return static_cast<metatype *>(this);
    case IODevice::Type: return static_cast<IODevice *>(this);
    default: return 0;
    }
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
