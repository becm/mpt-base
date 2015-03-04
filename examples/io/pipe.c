/* MPT child program I/O buffer */
#include <stdio.h>
#include <stdlib.h>

#include <signal.h>
#include <unistd.h>
#include <poll.h>

#include <mpt/queue.h>
#include <mpt/stream.h>

int main(int argc, char *argv[], char *env[])
{
	struct mpt_stream srm = MPT_STREAM_INIT;
	int mode;
	
	if ((mode = mpt_stream_pipe(&srm._info, argv[1], argv+1)) < 0) {
		perror("call");
		return 1;
	}
	mpt_stream_setmode(&srm, MPT_ENUM(StreamBuffer));
	
	while (mpt_stream_poll(&srm, POLLIN | POLLHUP, -1) >= 0) {
		fprintf(stderr, "%d\n", (int) srm._rd.len);
	}
	return 0;
}
