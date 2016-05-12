/* send MPT messages */
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(core.h)

int main(int argc, char *argv[])
{
	struct mpt_socket sock = MPT_SOCKET_INIT;
	char text[128];
	int len;
	
	if (argc < 2) {
		fprintf(stderr, "%s %s\n", argv[0], "<destination>");
		return 2;
	}
	if ((len = mpt_connect(&sock, argv[1], 0)) < 0) {
		perror("connect");
		return 1;
	}
	while (fgets(text, sizeof(text), stdin)) {
		size_t len = strlen(text);
		if (write(sock._id, text, len) < 0) {
			perror("write");
			return 1;
		}
	}
	return 0;
}
