/*!
 * create IO-connected process.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

#include "stream.h"

/*!
 * \ingroup mptStream
 * \brief start subprocess
 * 
 * Create process with input and output
 * connected to stream descriptor.
 * 
 * \param stream stream to read from
 * \param file   executable to start
 * \param argv   argument array
 * 
 * \return result of operation
 */
extern int mpt_stream_pipe(MPT_STRUCT(streaminfo) *stream, const char *file, char * const argv[])
{
	char *targ[2];
	int   in[2], out[2];
	pid_t pid;
	
	if (!file || (access(file, X_OK) < 0)) {
		errno = EINVAL;
		return -1;
	}
	if (pipe(in) < 0) {
		return -2;
	}
	if (pipe(out) < 0) {
		close(in[0]);
		close(in[1]);
		return -2;
	}
	if (_mpt_stream_setfile(stream, in[0], out[1]) < 0) {
		close(in[0]); close(in[1]);
		close(out[0]); close(out[1]);
		return -1;
	}
	
	switch (pid = fork()) {
	    case -1:
		close(in[0]);
		close(out[1]);
		break;
	    case 0:
		if (dup2(out[0], 0) < 0) {
			perror("dup2(input)");
			_exit(EXIT_FAILURE);
		}
		close(out[0]);
		close(out[1]);
		if (dup2(in[1], 1) < 0) {
			perror("dup2(output)");
			_exit(EXIT_FAILURE);
		}
		if (!argv) {
			targ[0] = (char*) file;
			targ[1] = 0;
			argv = targ;
		}
		close(in[0]);
		close(in[1]);
		execv(file, argv);
		perror(file);
		_exit(EXIT_FAILURE);
	    default:;
	}
	
	close(in[1]);
	close(out[0]);
	
	return 0;
}

