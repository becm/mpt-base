/*!
 * connect stream to address
 */

#ifndef _POSIX_C_SOURCE
# define _POSIX_C_SOURCE 200809L
#endif

#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <poll.h>

#include <fcntl.h>
#include <sys/stat.h>


#include <netinet/in.h>
#include <sys/un.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "convert.h"
#include "message.h"

static int socketSet(const char *where, MPT_STRUCT(fdmode) *mode, int (*operation)())
{
	int len, sock;
	
	/* unix socket */
	if (mode->family == AF_UNIX) {
		struct sockaddr_un addr;
		size_t len;
		
		if ((len = strlen(where)) >= sizeof(addr.sun_path)) {
			errno = ERANGE;
			return -2;
		}
		if ((sock = socket(AF_UNIX, mode->param.sock.type, mode->param.sock.proto)) < 0) {
			return sock;
		}
		addr.sun_family = AF_UNIX;
		memcpy(addr.sun_path, where, len+1);
		if (operation(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
			(void) close(sock);
			return -1;
		}
		mode->stream = MPT_SOCKETFLAG(Read) | MPT_SOCKETFLAG(Write);
		
		if (mode->param.sock.type == SOCK_STREAM) {
			mode->stream |= MPT_SOCKETFLAG(Stream);
		}
	}
	/* ip socket */
	else if (mode->family == AF_INET || mode->family == AF_INET6 || mode->family == AF_UNSPEC) {
		struct addrinfo hints, *res, *curr;
		char *port, *end, save = 0;
		
		if (mode->family != AF_INET && *where == '[') {
			if (!(end = strchr(++where, ']'))) {
				errno = ENOTSUP;
				return -2;
			}
			save = *end;
			*end = 0;
			port = (end[1] == ':') ? end+2 : 0;
		}
		else if ((end = port = strrchr(where, ':'))) {
			save = *end;
			*port++ = 0;
		}
		memset(&hints, 0, sizeof(hints));
		hints.ai_family   = mode->family;
		hints.ai_socktype = mode->param.sock.type;
		hints.ai_protocol = mode->param.sock.proto;
		
		if (!*where) {
			where = 0;
		}
		len = getaddrinfo(where, port, &hints, &res);
		
		if (end && save) {
			*end = save;
		}
		switch (len) {
		  case 0: break;
		  case EAI_AGAIN: return -2;
		  case EAI_SYSTEM: return -1;
		  default: return -3;
		}
		curr = res;
		sock = -1;
		
		while (curr) {
			mode->family           = curr->ai_family;
			mode->param.sock.type  = curr->ai_socktype;
			mode->param.sock.proto = curr->ai_protocol;
			
			if ((sock = socket(mode->family, mode->param.sock.type, mode->param.sock.proto)) < 0) {
				break;
			}
			if (operation(sock, curr->ai_addr, curr->ai_addrlen) >= 0) {
				break;
			}
			curr = curr->ai_next;
			
			errno = ENOPROTOOPT;
			(void) close(sock);
			sock = -1;
		}
		if (res) {
			freeaddrinfo(res);
		}
		mode->stream = MPT_SOCKETFLAG(Read) | MPT_SOCKETFLAG(Write);
		
		if (mode->param.sock.type == SOCK_STREAM) {
			mode->stream |= MPT_SOCKETFLAG(Stream);
		}
	}
	else {
		errno = EPROTONOSUPPORT;
		return -4;
	}
	return sock;
}

static int socketUnbind(int sockfd)
{
	struct sockaddr_un un;
	socklen_t len = sizeof(un);
	
	if (getsockname(sockfd, (struct sockaddr *) &un, &len) < 0) {
		return -1;
	}
	if (un.sun_family == AF_UNIX && len > sizeof(un.sun_family)) {
		return unlink(un.sun_path);
	}
	return 0;
}

extern int mpt_bind(MPT_STRUCT(socket) *sd, const char *where, const MPT_STRUCT(fdmode) *mode, int backlog)
{
	MPT_STRUCT(fdmode) info;
	int sock;
	
	if (!where) {
		if ((sock = sd->_id) < 0) {
			return 0;
		}
		(void) socketUnbind(sock);
		(void) close(sock);
		sd->_id = -1;
		return 1;
	}
	if (mode) {
		info = *mode;
	}
	else if ((sock = mpt_mode_parse(&info, where)) <= 0) {
		errno = EINVAL;
		return -1;
	} else {
		where += sock;
	}
	if (info.family < 0) {
		int res = -1;
		if (info.param.file.open & O_WRONLY) {
			errno = EINVAL;
			return -1;
		}
		if (info.param.file.open & O_CREAT) {
			if ((res = mkfifo(where, info.param.file.perm)) >= 0) {
				info.param.file.open &= ~(O_EXCL | O_CREAT);
			}
			else if (info.param.file.open & O_EXCL) {
				return res;
			}
		}
		if ((sock = open(where, info.param.file.open, info.param.file.perm)) < 0) {
			if (res >= 0) {
				unlink(where);
			}
			return sock;
		}
		info.stream = MPT_SOCKETFLAG(Read) | MPT_SOCKETFLAG(Stream);
	}
	else if ((sock = socketSet(where, &info, bind)) < 0) {
		return sock;
	}
	/* identify as listening socket */
	else if (info.stream == (MPT_SOCKETFLAG(Read) | MPT_SOCKETFLAG(Write) | MPT_SOCKETFLAG(Stream))) {
		if (backlog >= 0 && listen(sock, backlog) < 0) {
			(void) close(sock);
			return -1;
		}
		info.stream = 0;
	}
	if (sd->_id >= 0) {
		(void) close(sd->_id);
	}
	sd->_id = sock;
	
	return info.stream;
}

/*!
 * \ingroup mptCore
 * \brief socket connection
 * 
 * Connect socket descriptor to target address.
 * 
 * \param sd    socket descriptor
 * \param where connection target
 * \param mode  target mode
 * 
 * \return log operation result
 */
extern int mpt_connect(MPT_STRUCT(socket) *sd, const char *where, const MPT_STRUCT(fdmode) *mode)
{
	MPT_STRUCT(fdmode) info;
	int type, sock;
	
	if (!where) {
		if (sd->_id >= 0) {
			(void) close(sd->_id);
			return 1;
		}
		return 0;
	}
	if (mode) {
		info = *mode;
	}
	else if ((type = mpt_mode_parse(&info, where)) <= 0) {
		errno = EINVAL;
		return -1;
	} else {
		where += type;
	}
	/* one-way type connection */
	if (info.family < 0) {
		if (info.param.file.open & O_RDWR) {
			errno = EINVAL;
			return -1;
		}
		if ((sock = open(where, info.param.file.open, info.param.file.perm)) < 0) {
			return sock;
		}
		info.stream  = (info.param.file.open & O_WRONLY) ? MPT_SOCKETFLAG(Write) : MPT_SOCKETFLAG(Read);
		info.stream |= MPT_SOCKETFLAG(Stream);
	}
	else if ((sock = socketSet(where, &info, connect)) < 0) {
		return sock;
	}
	if (sd->_id >= 0) {
		(void) close(sd->_id);
	}
	sd->_id = sock;
	return info.stream;
}

