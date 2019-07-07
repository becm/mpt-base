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

#include "../mptio/stream.h"
#include "../mptio/connection.h"

#include "io.h"

__MPT_NAMESPACE_BEGIN

// socket data operations
socket::~socket()
{ mpt_bind(this, 0, 0, 0); }

bool socket::bind(const char *addr, int listen)
{
	return (mpt_bind(this, addr, 0, listen) < 0) ? false : true;
}

bool socket::set(convertable &src)
{
	socket tmp;
	const char *dst = 0;
	if (src.convert('s', &dst) < 0) {
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
io::socket::socket(struct ::mpt::socket *from)
{
	if (!from) {
		return;
	}
	*static_cast<::mpt::socket *>(this) = *from;
	new (from) ::mpt::socket;
}
io::socket::~socket()
{ }

int io::socket::assign(const value *val)
{
	return (::mpt::socket::set(val)) ? (val ? 1 : 0) : BadOperation;
}

reference<io::stream> io::socket::accept()
{
	int sock;
	
	if ((sock = ::accept(_id, 0, 0)) < 0) {
		return 0;
	}
	streaminfo info;
	
	if (_mpt_stream_setfile(&info, sock, sock) < 0) {
		return 0;
	}
	info.set_flags(::mpt::stream::Buffer);
	
	class io::stream *s = new io::stream(&info);
	
	return s;
}

__MPT_NAMESPACE_END
