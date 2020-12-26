/*!
 * test MPT poll compound
 */

#include <stdio.h>

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(message.h)

#include MPT_INCLUDE(notify.h)

static int print_message(mpt::input *in, mpt::event *ev)
{
	mpt::message msg;
	mpt::msgtype mt(0);
	char buf[128];
	int len, total = 0;
	
	if (!ev || !ev->msg) {
		return -1;
	}
	
	msg = *ev->msg;
	total = msg.length();
	
	int file = -1;
	in->convert(mpt::TypeUnixSocket, &file);
	msg.read(sizeof(mt), &mt);
	printf("file: %d, size: %d, { cmd: %d, arg: %d }\n",
	       file, total, mt.cmd, mt.cmd);
	
	while ((len = msg.read(sizeof(buf) - 1, &buf))) {
		buf[len] = 0;
		if (strstr(buf, "exit")) {
			puts("\nexiting");
			return mpt::event::Terminate;
		}
		fputs(buf, stdout);
	}
	fputc('\n', stdout);
	return 0;
}

extern int main(int argc, char *argv[])
{
	mpt::notify no;
	int arg;
	
	mtrace();
	
	for (arg = 1; arg < argc; arg++) {
		if (mpt_notify_connect(&no, argv[arg]) < 0) {
			perror("add");
			return 1;
		}
	}
	
	while (no.used()) {
		mpt::input *in;
		while ((in = no.next())) {
			while ((arg = in->dispatch((mpt::event_handler_t) print_message, in)) > 0) {
				if (arg & mpt::event::Terminate) {
					return 0;
				}
				if (!(arg & mpt::event::Retry)) {
					break;
				}
			}
		}
		if ((arg = no.wait(-1, -1)) < 0) {
			perror("wait");
			break;
		}
	}
	
	return 0;
}

