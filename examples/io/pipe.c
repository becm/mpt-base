/* MPT child program I/O buffer */
#include <stdio.h>
#include <stdlib.h>

#include <signal.h>
#include <unistd.h>
#include <poll.h>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(queue.h)
#include MPT_INCLUDE(stream.h)

int main(int argc, char * const argv[])
{
	struct mpt_stream srm = MPT_STREAM_INIT;
	int mode;
	
	if (argc < 2) {
		fprintf(stderr, "%s %s [%s]\n", *argv, "<executable>", "<args>");
		return 1;
	}
	if ((mode = mpt_stream_pipe(&srm._info, argv[1], argv+1)) < 0) {
		perror("call");
		return 1;
	}
	mpt_stream_setmode(&srm, MPT_ENUM(StreamBuffer));
	
	while (mpt_stream_poll(&srm, POLLIN | POLLHUP, -1) > 0) {
		fprintf(stderr, "%d (%d)\n", (int) srm._rd.data.len, (int) srm._rd.data.max);
	}
	fwrite(srm._rd.data.base, srm._rd.data.len, 1, stdout);
	mpt_stream_close(&srm);
	return 0;
}
