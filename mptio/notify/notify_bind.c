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

#include "connection.h"
#include "stream.h"
#include "types.h"

#include "notify.h"

struct socketInput {
	MPT_INTERFACE(input) _in;
	MPT_STRUCT(socket)  sock;
	MPT_STRUCT(notify) *no;
	int nl;
};

/* metatype interface */
static int socket_conv(MPT_INTERFACE(convertable) *val, MPT_TYPE(type) type, void *ptr)
{
	struct socketInput *sd = (void *) val;
	const MPT_STRUCT(named_traits) *traits = mpt_input_type_traits();
	MPT_TYPE(type) me;
	
	if (!traits) {
		me = MPT_ENUM(TypeMetaPtr);
	}
	else if (type == (me = traits->type)) {
		if (ptr) *((void **) ptr) = &sd->_in;
		return MPT_ENUM(TypeUnixSocket);
	}
	if (!type) {
		static const char fmt[] = { MPT_ENUM(TypeUnixSocket), 0 };
		if (ptr) *((const char **) ptr) = fmt;
		return me;
	}
	if (type == MPT_ENUM(TypeMetaPtr)) {
		if (ptr) *((void **) ptr) = &sd->_in;
		return MPT_ENUM(TypeUnixSocket);
	}
	if (type == MPT_ENUM(TypeUnixSocket)) {
		if (ptr) *((MPT_STRUCT(socket) *) ptr) = sd->sock;
		return me;
	}
	return MPT_ERROR(BadType);
}
/* reference interface */
static void socket_unref(MPT_INTERFACE(metatype) *mt)
{
	struct socketInput *sd = (void *) mt;
	mpt_bind(&sd->sock, 0, 0, 0);
	free(sd);
}
static uintptr_t socket_addref(MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
static MPT_INTERFACE(metatype) *socket_clone(const MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
/* input interface */
static int socket_next(MPT_INTERFACE(input) *in, int what)
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
		srm->_vptr->meta.unref((void *) srm);
		return -1;
	}
	/* single connection mode */
	if (sd->nl < 0) {
		return -2;
	}
	return 0;
}
static int socket_dispatch(MPT_INTERFACE(input) *in, MPT_TYPE(event_handler) cmd, void *arg)
{
	(void) in;
	(void) cmd;
	(void) arg;
	return MPT_ERROR(BadOperation);
}

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
 * \return socket port
 */
extern int mpt_notify_bind(MPT_STRUCT(notify) *no, const char *dest, int nl)
{
	static const MPT_INTERFACE_VPTR(input) socketInput = {
		{ { socket_conv },
		  socket_unref,
		  socket_addref,
		  socket_clone
		},
		socket_next,
		socket_dispatch
	};
	MPT_STRUCT(fdmode) mode;
	MPT_STRUCT(socket) sock = MPT_SOCKET_INIT;
	struct socketInput *in;
	int type, port;
	
	if (!no || !dest) {
		return MPT_ERROR(BadArgument);
	}
	/* insist on tcp listening socket */
	if ((type = mpt_mode_parse(&mode, dest)) < 0) {
		return type;
	}
	/* default to stream socket */
	if (!type) {
		if ((port = mpt_bind(&sock, dest, 0, nl)) < 0) {
			return port;
		}
	}
	/* no file or packet modes allowed */
	else if ((mode.family < 0) || (mode.param.sock.type != SOCK_STREAM)) {
		return MPT_ERROR(BadType);
	}
	/* create listening socket */
	else if ((port = mpt_bind(&sock, dest+type, &mode, nl < 0 ? 0 : nl)) < 0) {
		return port;
	}
	if (!(in = malloc(sizeof(*in)))) {
		close(sock._id);
		return MPT_ERROR(BadOperation);
	}
	in->_in._vptr = &socketInput;
	in->sock = sock;
	in->no = no;
	in->nl = nl;
	
	if ((type = mpt_notify_add(no, POLLIN, (void *) in)) >= 0) {
		return port;
	}
	close(sock._id);
	free(in);
	return type;
}
