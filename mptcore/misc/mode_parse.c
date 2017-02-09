/*!
 * parse path to address type and data.
 */
#include <stdlib.h>

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <strings.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>

#include "convert.h"

#include "../mptio/stream.h"

static int socketMode(MPT_STRUCT(fdmode) *mode, const char *path)
{
	size_t len;
	
	if (!path) {
		return -1;
	}
	/* force unix socket */
	if (!strncasecmp(path, "socket", len = 6) || !strncasecmp(path, "sock", len = 4)) {
		if (path[len] == ':' && (path[len+1] == '/' || path[len+1] == '.')) {
			mode->family = AF_UNIX;
			mode->param.sock.type  = isupper(*path) ? SOCK_STREAM : SOCK_DGRAM;
			mode->param.sock.proto = 0;
		} else {
			mode->family = AF_UNSPEC;
			if (isupper(*path)) {
				mode->param.sock.type  = SOCK_STREAM;
				mode->param.sock.proto = IPPROTO_TCP;
			} else {
				mode->param.sock.type  = SOCK_DGRAM;
				mode->param.sock.proto = IPPROTO_UDP;
			}
		}
		mode->stream = isupper(*path) ? MPT_ENUM(StreamBuffer) : 0;
	}
	/* force unix socket */
	else if (!strncasecmp(path, "unix", len = 4)) {
		mode->family = AF_UNIX;
		mode->param.sock.type  = isupper(*path) ? SOCK_STREAM : SOCK_DGRAM;
		mode->param.sock.proto = 0;
		mode->stream = isupper(*path) ? MPT_ENUM(StreamBuffer) : 0;
	}
	/* name is ipv4 address */
	else if (!strncasecmp(path, "ip4", len = 3) || !strncasecmp(path, "ipv4", len = 5)) {
		mode->family = AF_INET;
		mode->param.sock.type  = isupper(*path) ? SOCK_STREAM : SOCK_DGRAM;
		mode->param.sock.proto = 0;
		mode->stream = isupper(*path) ? MPT_ENUM(StreamBuffer) : 0;
	}
	/* name is ipv4 tcp address */
	else if (!strncasecmp(path, "tcp4", len = 4)) {
		mode->family = AF_INET;
		mode->param.sock.type  = SOCK_STREAM;
		mode->param.sock.proto = IPPROTO_TCP;
		mode->stream = MPT_ENUM(StreamBuffer);
	}
	/* name is ipv4 udp address */
	else if (!strncasecmp(path, "udp4", len = 4)) {
		mode->family = AF_INET;
		mode->param.sock.type  = SOCK_DGRAM;
		mode->param.sock.proto = IPPROTO_UDP;
		mode->stream = 0;
	}
#ifdef IPPROTO_SCTP
	/* name is ipv4 sctp address */
	else if (!strncasecmp(path, "sctp4", len = 5)) {
		mode->family = AF_INET;
		mode->param.sock.type  = isupper(*path) ? SOCK_STREAM : SOCK_SEQPACKET;
		mode->param.sock.proto = IPPROTO_SCTP;
		mode->stream = isupper(*path) ? MPT_ENUM(StreamBuffer) : 0;
	}
#endif
	/* name is ipv6 address */
	else if (!strncasecmp(path, "ip6", len = 3) || !strncasecmp(path, "ipv6", len = 4)) {
		mode->family = AF_INET6;
		mode->param.sock.type  = isupper(*path) ? SOCK_STREAM : SOCK_DGRAM;
		mode->param.sock.proto = 0;
		mode->stream = isupper(*path) ? MPT_ENUM(StreamBuffer) : 0;
	}
	/* name is ipv6 tcp address */
	else if (!strncasecmp(path, "tcp6", len = 4)) {
		mode->family = AF_INET6;
		mode->param.sock.type  = SOCK_STREAM;
		mode->param.sock.proto = IPPROTO_TCP;
		mode->stream = MPT_ENUM(StreamBuffer);
	}
	/* name is ipv6 udp address */
	else if (!strncasecmp(path, "udp6", len = 4)) {
		mode->family = AF_INET6;
		mode->param.sock.type  = SOCK_DGRAM;
		mode->param.sock.proto = IPPROTO_UDP;
		mode->stream = 0;
	}
#ifdef IPPROTO_SCTP
	/* name is ipv6 sctp address */
	else if (!strncasecmp(path, "sctp6", len = 5)) {
		mode->family = AF_INET6;
		mode->param.sock.type  = isupper(*path) ? SOCK_STREAM : SOCK_SEQPACKET;
		mode->param.sock.proto = IPPROTO_SCTP;
		mode->stream = isupper(*path) ? MPT_ENUM(StreamBuffer) : 0;
	}
#endif
	/* name is unspecified ip address */
	else if (!strncasecmp(path, "ip", len = 2)) {
		mode->family = AF_UNSPEC;
		mode->param.sock.type  = isupper(*path) ? SOCK_STREAM : SOCK_DGRAM;
		mode->param.sock.proto = 0;
		mode->stream = isupper(*path) ? MPT_ENUM(StreamBuffer) : 0;
	}
	/* name is unspecified tcp address */
	else if (!strncasecmp(path, "tcp", len = 3)) {
		mode->family = AF_UNSPEC;
		mode->param.sock.type  = SOCK_STREAM;
		mode->param.sock.proto = IPPROTO_TCP;
		mode->stream = MPT_ENUM(StreamBuffer);
	}
	/* name is unspecified udp address */
	else if (!strncasecmp(path, "udp", len = 3)) {
		mode->family = AF_UNSPEC;
		mode->param.sock.type  = SOCK_DGRAM;
		mode->param.sock.proto = IPPROTO_UDP;
		mode->stream = 0;
	}
#ifdef IPPROTO_SCTP
	/* name is unspecified sctp address */
	else if (!strncasecmp(path, "sctp", len = 4)) {
		mode->family = AF_UNSPEC;
		mode->param.sock.type  = isupper(*path) ? SOCK_STREAM : SOCK_SEQPACKET;
		mode->param.sock.proto = IPPROTO_SCTP;
		mode->stream = isupper(*path) ? MPT_ENUM(StreamBuffer) : 0;
	}
#endif
	else {
		return MPT_ERROR(BadArgument);
	}
	mode->param.sock.port = 0;
	mode->stream |= MPT_ENUM(StreamRdWr);
	return len;
}

static long getPerm(const char **pos)
{
	/* default permissions: 0666 */
	long val = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
	const char *end;
	
	if (pos && (end = *pos) && *end == ',') {
		if (isspace(*(++end))) {
			return MPT_ERROR(BadArgument);
		}
		val = strtol(end, (char **) pos, 8);
		
		if (*pos == end) {
			return MPT_ERROR(BadType);
		}
		if (val < 0400 || val > 07777) {
			return MPT_ERROR(BadValue);
		}
	}
	return val;
}

static void lineSeparator(MPT_STRUCT(fdmode) *mode, const char **pos)
{
	switch (**pos) {
	  case 'N': case 'W':
		mode->stream |= MPT_ENUM(StreamFlushLine);
	  case 'n': case 'w':
		mode->lsep = MPT_ENUM(NewlineNet);
		break;
	  case 'U':
		mode->stream |= MPT_ENUM(StreamFlushLine);
	  case 'u':
		mode->lsep = MPT_ENUM(NewlineUnix);
		break;
	  case 'M':
		mode->stream |= MPT_ENUM(StreamFlushLine);
	  case 'm':
		mode->lsep = MPT_ENUM(NewlineMac);
		break;
	  default:
		return;
	}
	++(*pos);
}

/*!
 * \ingroup mptCore
 * \brief set file mode
 * 
 * Parse socket or local file mode description.
 * 
 * \param[out] smode  file mode information
 * \param      mode   mode description
 * 
 * \return new metatype instance
 */
extern int mpt_mode_parse(MPT_STRUCT(fdmode) *smode, const char *mode)
{
	MPT_STRUCT(fdmode) sm, def;
	const char *end;
	long val;
	sm.family = -1;
	sm.lsep = mpt_newline_native();
	sm.param.file.open = 0;
	sm.param.file.perm = 0;
	sm.stream = 0;
	
	if (!mode) {
		sm.param.file.open = O_RDONLY;
		
		*smode = sm;
		return 0;
	}
	def = sm;
	
	/* try socket types */
	if ((val = socketMode(smode, mode)) > 0
	     && (!mode[val] || mode[val++] == ':')) {
		return val;
	}
	/* select file mode */
	end = mode+1;
	switch (*mode) {
		case '\0':
			*smode = sm;
			return 0;
		case ':':
			sm.param.file.open |= O_RDONLY;
			*smode = sm;
			return 1;
		case 'M':
			sm.stream |= MPT_ENUM(StreamForceMap);
		case 'm':
			sm.stream |= MPT_ENUM(StreamReadMap);
			sm.param.file.open |= O_RDONLY;
			lineSeparator(&sm, &end);
			break;
		case 'R':
			sm.param.file.open |= O_NONBLOCK;
		case 'r':
			lineSeparator(&sm, &end);
			if (*end == '+') {
				sm.param.file.open |= O_RDWR;
				sm.stream |= MPT_ENUM(StreamRdWr) | MPT_ENUM(StreamBuffer);
				++end;
			}
			else {
				sm.param.file.open |= O_RDONLY;
				sm.stream |= MPT_ENUM(StreamRead) | MPT_ENUM(StreamReadBuf);
			}
			break;
		case 'A':
			sm.param.file.open |= O_NONBLOCK;
		case 'a':
			lineSeparator(&sm, &end);
			sm.param.file.open |= O_APPEND | O_CREAT;
			if (*end == '+') {
				sm.stream |= MPT_ENUM(StreamRdWr) | MPT_ENUM(StreamWriteBuf);
				sm.param.file.open |= O_RDWR;
				++end;
			}
			else {
				sm.stream |= MPT_ENUM(StreamWrite) | MPT_ENUM(StreamWriteBuf);
				sm.param.file.open |= O_WRONLY;
			}
			break;
		case 'W':
			sm.param.file.open |= O_NONBLOCK;
		case 'w':
			lineSeparator(&sm, &end);
			sm.param.file.open |= O_CREAT;
			if (*end == '+') {
				sm.stream    |= MPT_ENUM(StreamRdWr) | MPT_ENUM(StreamBuffer);
				sm.param.file.open |= O_RDWR;
				++end;
			}
			else {
				sm.stream |= MPT_ENUM(StreamWrite) | MPT_ENUM(StreamWriteBuf);
				sm.param.file.open |= O_WRONLY;
			}
			break;
		default:
			*smode = sm;
			return 0;
	}
	if (sm.param.file.open & (O_RDWR | O_WRONLY)) {
		if ((val = getPerm(&end)) < 0) {
			return val;
		}
		sm.param.file.perm = val;
	}
	
	if (!*end || *(end++) == ':') {
		*smode = sm;
		return end - mode;
	}
	*smode = def;
	return 0;
}

