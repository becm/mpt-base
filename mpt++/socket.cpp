/*!
 * MPT C++ socket implementation
 */

#include <new>

#include <unistd.h>
#include <fcntl.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "meta.h"

#include "stream.h"

#include "connection.h"

__MPT_NAMESPACE_BEGIN

// socket data operations
socket::~socket()
{ mpt_bind(this, 0, 0, 0); }

bool socket::bind(const char *addr, int listen)
{
    return (mpt_bind(this, addr, 0, listen) < 0) ? false : true;
}

bool socket::set(metatype &src)
{
    socket tmp;
    const char *dst = 0;
    if (src.conv('s', &dst) < 0) {
        return false;
    }
    if (dst && mpt_connect(&tmp, dst, 0) < 0) {
        return false;
    }
    if (_id) {
        (void) close(_id);
    }
    _id = tmp._id;
    return true;
}
bool socket::set(const value *val)
{
    if (!val) {
        mpt_bind(this, 0, 0, 0);
        return 0;
    }
    if (!val->fmt) {
        socket tmp;
        const char *dst;
        if (!(dst = val->string())
         || mpt_connect(&tmp, dst, 0) < 0) {
            return false;
        }
        if (_id) {
            (void) close(_id);
        }
        _id = tmp._id;
        return true;
    }
    return false;
}

// socket class
Socket::Socket(struct socket *from)
{
    if (!from) return;
    *static_cast<socket *>(this) = *from;
    new (from) socket;
}
Socket::~Socket()
{ }

int Socket::assign(const value *val)
{
    return (socket::set(val)) ? (val ? 1 : 0) : BadOperation;
}

Reference<Stream> Socket::accept()
{
    int sock;

    if ((sock = ::accept(_id, 0, 0)) < 0) {
        return 0;
    }
    streaminfo info;

    if (_mpt_stream_setfile(&info, sock, sock) < 0) {
        return 0;
    }
    info.setFlags(stream::Buffer);

    class Stream *s = new class Stream(&info);

    return s;
}

__MPT_NAMESPACE_END
