/*!
 * new socket bound to notifier.
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <poll.h>

#include <sys/types.h>
#include <sys/socket.h>

#include "convert.h"
#include "event.h"

#include "stream.h"
#include "notify.h"

struct socketInput {
	MPT_INTERFACE(input) _in;
	MPT_STRUCT(socket)    sock;
	MPT_STRUCT(notify)   *no;
};

static void socketUnref(MPT_INTERFACE(unrefable) *in)
{
	struct socketInput *sd = (void *) in;
	mpt_bind(&sd->sock, 0, 0, 0);
	free(sd);
}
static int socketNext(MPT_INTERFACE(input) *in, int what)
{
	static int flags = MPT_STREAMFLAG(RdWr) | MPT_STREAMFLAG(Buffer);
	MPT_STRUCT(socket) sock;
	struct socketInput *sd = (void *) in;
	MPT_INTERFACE(input) *srm;
	
	if (!(what & POLLIN)) return 0;
	
	if ((sock._id = accept(sd->sock._id, 0, 0)) < 0) {
		return -1;
	}
	if (!(srm = mpt_stream_input(&sock, flags, MPT_ENUM(EncodingCobs), sizeof(uint16_t)))) {
		(void) close(sock._id);
		return -1;
	}
	
	if (mpt_notify_add(sd->no, POLLIN, srm) < 0) {
		srm->_vptr->ref.unref((void *) srm);
		return -1;
	}
	return 0;
}
static int socketDispatch(MPT_INTERFACE(input) *in, MPT_TYPE(EventHandler) cmd, void *arg)
{
	(void) in;
	(void) cmd;
	(void) arg;
	return -1;
}
static int socketFile(MPT_INTERFACE(input) *in)
{
	struct socketInput *sd = (void *) in;
	return sd->sock._id;
}
static const MPT_INTERFACE_VPTR(input) socketInput = {
	{ socketUnref },
	socketNext,
	socketDispatch,
	socketFile
};

/*!
 * \ingroup mptNotify
 * \brief new socket
 * 
 * Create bound socket on notifier.
 * 
 * \param no    notification descriptor
 * \param dest  bind address
 * \param nl    listening queue length
 * 
 * \return socket descriptor
 */
extern int mpt_notify_bind(MPT_STRUCT(notify) *no, const char *dest, int nl)
{
	MPT_STRUCT(fdmode) mode;
	MPT_STRUCT(socket) sock = MPT_SOCKET_INIT;
	struct socketInput *in;
	int type;
	
	if (!no || !dest) {
		errno = EFAULT;
		return -1;
	}
	/* insist on tcp listening socket */
	if ((type = mpt_mode_parse(&mode, dest)) <= 0
	    || mode.family < 0
	    || mode.param.sock.type != SOCK_STREAM) {
		errno = EINVAL;
		return -1;
	}
	if ((type = mpt_bind(&sock, dest+type, &mode, nl)) < 0) {
		return -1;
	}
	if (!(in = malloc(sizeof(*in)))) {
		close(sock._id);
		return -1;
	}
	in->_in._vptr = &socketInput;
	in->sock = sock;
	in->no = no;
	
	if (mpt_notify_add(no, POLLIN, &in->_in) >= 0) {
		return sock._id;
	}
	close(sock._id);
	free(in);
	return -2;
}
