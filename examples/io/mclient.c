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
	struct mpt_outdata out = MPT_OUTDATA_INIT;
	struct mpt_stream *srm;
	int flg;
	char txt[8];
	
	if (argc < 2) {
		fputs(*argv, stderr);
		fputs(" <target>\n", stderr);
		return 1;
	}
	if ((flg = mpt_outdata_connect(&out, argv[1], 0)) < 0) {
		perror("connect");
		return 1;
	}
	if (flg & MPT_ENUM(SocketStream) && (srm = out._buf)) {
		srm->_wd._enc = mpt_encode_cobs;
	}
	
	while (fgets(txt, sizeof(txt), stdin)) {
		const char *end;
		size_t len = sizeof(txt);
		
		if ((end = memchr(txt, 0, len))) len = end - txt;
		
		if (!len) continue;
		
		/* add zero id for bidirectional */
		if ((flg & MPT_ENUM(SocketRead)) && !(out.state & MPT_ENUM(OutputActive))) {
			static const uint16_t mid = 0;
			if (mpt_outdata_push(&out, sizeof(mid), &mid) < 0) {
				break;
			}
		}
		/* push text */
		if (txt[len-1] != '\n') {
			if (mpt_outdata_push(&out, len, txt) < 0) {
				perror("transmit part");
				break;
			}
			continue;
		}
		if (mpt_outdata_push(&out, --len, txt) < 0) {
			perror("transmit last");
			break;
		}
		if (len && mpt_outdata_push(&out, 0, 0) < 0) {
			perror("transmit separation");
			break;
		}
	}
	mpt_outdata_fini(&out);
	return 0;
}

