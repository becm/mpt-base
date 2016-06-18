/*!
 * test MPT poll compound
 */

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <poll.h>
#include <sys/uio.h>

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

#include <mpt/array.h>
#include <mpt/message.h>
#include <mpt/event.h>

#include <mpt/stream.h>
#include <mpt/output.h>

#include <mpt/notify.h>

static int printMessage(void *fd, MPT_STRUCT(event) *ev)
{
	if (!ev) {
		return 0;
	}
	if (ev->msg) {
		MPT_STRUCT(message) msg = *ev->msg;
		fwrite(msg.base, msg.used, 1, fd);
		while (msg.clen--) {
			fwrite(msg.cont->iov_base, msg.cont->iov_len, 1, fd);
			++msg.cont;
		}
	}
	if (ev->reply.set) {
		ev->reply.set(ev->reply.context, ev->msg);
	}
	return 0;
}

extern int main(int argc, char *argv[])
{
	MPT_STRUCT(notify) no = MPT_NOTIFY_INIT;
	MPT_STRUCT(dispatch) dsp = MPT_DISPATCH_INIT;
	int arg;
	
	mtrace();
	
	no._disp.cmd = (MPT_TYPE(EventHandler)) mpt_dispatch_emit;
	no._disp.arg = &dsp;
	
	dsp._err.cmd = printMessage;
	dsp._err.arg = stdout;
	
	dsp._out = mpt_output_new(&no);
	mpt_dispatch_context(&dsp._ctx);
	
	for (arg = 1; arg < argc; arg++) {
		MPT_STRUCT(socket) sock = MPT_SOCKET_INIT;
		MPT_STRUCT(fdmode) mode;
		MPT_INTERFACE(input) *in;
		int flg;
		
		if ((flg = mpt_mode_parse(&mode, argv[arg])) < 0) {
			perror("flags");
			return 1;
		}
		if ((flg = mpt_connect(&sock, argv[arg]+flg, &mode) < 0)) {
			perror("connect");
			return 1;
		}
		if (!(in = mpt_stream_input(&sock, mode.stream, 0, 0))) {
			perror("input");
			return 1;
		}
		if (mpt_notify_add(&no, POLLIN, in) < 0) {
			perror("add");
			return 2;
		}
	}
	arg = mpt_loop(&no);
	
	mpt_dispatch_fini(&dsp);
	mpt_notify_fini(&no);
	
	return arg < 0 ? 1 : 0;
}

