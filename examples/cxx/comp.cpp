/*!
 * test MPT poll compound
 */

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

#include <mpt/array.h>
#include <mpt/message.h>
#include <mpt/event.h>

#include <mpt/notify.h>

static int printMessage(mpt::input *in, mpt::event *ev)
{
	mpt::message msg;
	mpt::msgtype mt(0);
	char buf[128];
	int len, total = 0;
	
	if (!ev || !ev->msg) return -1;
	
	msg = *ev->msg;
	total = msg.length();
	
	msg.read(sizeof(mt), &mt);
	printf("file: %d, size: %d, { cmd: %d, arg: %d }\n",
	       in->_file(), total, mt.cmd, mt.cmd);
	
	while ((len = msg.read(sizeof(buf)-1, &buf))) {
		buf[sizeof(buf)-1] = 0;
		if (strstr(buf, "exit")) {
			puts("\nexiting");
			return mpt::EventTerminate;
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
			while ((arg = in->dispatch((mpt::EventHandler) printMessage, in)) > 0) {
				if (arg & mpt::EventTerminate) {
					return 0;
				}
				if (!(arg & mpt::EventRetry)) {
					break;
				}
			}
		}
		if ((arg = no.wait(-1, -1)) < 0) {
			perror("wait"); break;
		}
	}
	
	return 0;
}

