#include <stdio.h>
#include <string.h>
#include <poll.h>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(array.h)
#include MPT_INCLUDE(convert.h)
#include MPT_INCLUDE(message.h)

#include MPT_INCLUDE(queue.h)
#include MPT_INCLUDE(stream.h)

#include MPT_INCLUDE(output.h)

int main(int argc, char *argv[])
{
	struct mpt_connection con = MPT_CONNECTION_INIT;
	int flg;
	char txt[8];
	
	if (argc < 2) {
		fputs(*argv, stderr);
		fputs(" <target>\n", stderr);
		return 1;
	}
	if ((flg = mpt_connection_open(&con, argv[1], 0)) < 0) {
		perror("connect");
		return 1;
	}
	con.out._idlen = 2;
	
	while (fgets(txt, sizeof(txt), stdin)) {
		const char *end;
		size_t len = sizeof(txt);
		
		/* line end in buffer */
		if ((end = memchr(txt, 0, len))) {
			len = end - txt;
		}
		if (!len) continue;
		
		/* push text */
		if (txt[len-1] != '\n') {
			if (mpt_connection_push(&con, len, txt) < 0) {
				perror("transmit part");
				break;
			}
			continue;
		}
		if (mpt_connection_push(&con, --len, txt) < 0) {
			perror("transmit last");
			break;
		}
		if (len && mpt_connection_push(&con, 0, 0) < 0) {
			perror("transmit separation");
			break;
		}
	}
	mpt_connection_fini(&con);
	return 0;
}

