/*!
 * test MPT stream read
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include <poll.h>

#include <sys/uio.h>
#include <sys/socket.h>

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(array.h)
#include MPT_INCLUDE(queue.h)

#include MPT_INCLUDE(connection.h)
#include MPT_INCLUDE(stream.h)

/* easyest/general way to access envionment */
extern char **environ;

int main(int argc, char *argv[])
{
	struct mpt_socket sock = MPT_SOCKET_INIT;
	struct mpt_stream srm  = MPT_STREAM_INIT;
	int type;
	
	mtrace();
	
	if (argc < 2) {
		fputs("missing socket description\n", stderr);
		return 1;
	}
	if ((type = mpt_bind(&sock, argv[1], 0, 0)) < 0) {
		perror("bind");
		return 2;
	}
	if (type) {
		_mpt_stream_setfile(&srm._info, sock._id, -1);
	}
	else {
		socklen_t tlen;
		int fd;
		if ((fd = accept(sock._id, NULL, 0)) < 0) {
			perror("accept");
			return 3;
		}
		else {
			_mpt_stream_setfile(&srm._info, fd, fd);
			mpt_bind(&sock, 0, 0, 0);
		}
		mpt_stream_setmode(&srm, MPT_ENUM(StreamBuffer));
		
		tlen = sizeof(type);
		if (getsockopt(fd, SOL_SOCKET, SO_TYPE, &type, &tlen) < 0) {
			perror("getsockopt");
			return 4;
		}
	}
	while (mpt_stream_poll(&srm, POLLIN, -1) > 0) {
		const void *base;
		size_t low, len;
		
		if (!(len = srm._rd.data.len)) {
			continue;
		}
		base = mpt_queue_data(&srm._rd.data, &low);
		fwrite(base, 1, low, stdout);
		
		if ((len -= low)) {
			fwrite(srm._rd.data.base, 1, len, stdout);
		}
		srm._rd.data.len = 0;
		
		/* udp: exit after first message */
		if (type == SOCK_DGRAM) {
			break;
		}
	}
	
	mpt_stream_close(&srm);
	
	return 0;
}

