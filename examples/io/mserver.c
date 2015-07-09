
#include <stdio.h>

#include <string.h>
#include <sys/uio.h>

#include <mpt/array.h>
#include <mpt/message.h>
#include <mpt/event.h>

#include <mpt/notify.h>

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

static int wrap_io(void *fd, struct mpt_event *ev)
{
	struct mpt_message msg;
	int i = 0;
	
	if (!ev || !(ev->msg)) return -1;
	
	msg = *ev->msg;
	
	do {
		fprintf(stderr, "%s %i:", "part", i++);
		if (msg.used) {
			fwrite(msg.base, 1, msg.used, fd);
		}
		fputc('\n', fd);
		if (!msg.clen--) break;
		msg.base = msg.cont->iov_base;
		msg.used = msg.cont->iov_len;
		++msg.cont;
	} while (1);
	return 0;
}

int main(int argc, char *argv[])
{
	struct mpt_notify no;
	uint32_t mincon;
	
	mtrace();
	
	if (argc < 2) {
		fputs("need connection argument\n", stderr);
		return 1;
	}
	mpt_notify_init(&no);
	
	/* create socket on notification descriptor */
	if (mpt_notify_bind(&no, argv[1], 2) >= 0) {
		mincon = 1;
	}
	else if (mpt_notify_connect(&no, argv[1]) >= 0) {
		mincon = 0;
	}
	else {
		perror("psocket");
		return 2;
	}
	do {
		struct mpt_input *in;
		int evcnt;
		
		/* wait for connection/input */
		if ((evcnt = mpt_notify_wait(&no, -1, -1)) < 0) {
			perror("poll_wait");
			break;
		}
		if (!evcnt) continue;
		
		/* handle existing events */
		while ((in = mpt_notify_next(&no))) {
			/* read data if not socket */
			while (in->_vptr->dispatch(in, wrap_io, stdout) > 0) {
				puts("handled event");
			}
		}
	/* exit with last connection */
	} while (no._fdused > mincon);
	
	mpt_notify_fini(&no);
	
	return 0;
}
