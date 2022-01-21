/*!
 * set initial parameter for output descriptor
 */

#ifndef _POSIX_C_SOURCE
# define _POSIX_C_SOURCE 200809L
#endif

#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <poll.h>

#include "meta.h"
#include "message.h"
#include "types.h"

#include "object.h"
#include "output.h"

#include "stream.h"
#include "notify.h"
#include "connection.h"

MPT_STRUCT(out_data) {
	MPT_INTERFACE(input)  _in;
	
	MPT_INTERFACE(object) _obj;
	MPT_INTERFACE(output) _out;
	MPT_INTERFACE(logger) _log;
	
	MPT_STRUCT(refcount) _ref;
	
	MPT_STRUCT(connection) con;
};

static int remote_infile(const MPT_STRUCT(out_data) *od)
{
	MPT_STRUCT(stream) *srm;
	if (MPT_socket_active(&od->con.out.sock)) {
		return od->con.out.sock._id;
	}
	if (!(srm = (void *) od->con.out.buf._buf)) {
		return -1;
	}
	return _mpt_stream_fread(&srm->_info);
}
/* convertable interface */
static int remoteConv(MPT_INTERFACE(convertable) *val, int type, void *ptr)
{
	const MPT_STRUCT(out_data) *od = MPT_baseaddr(out_data, val, _in);
	const MPT_STRUCT(named_traits) *traits = mpt_input_type_traits();
	int me;
	
	if (!traits) {
		me = MPT_ENUM(TypeMetaPtr);
	}
	else if (type == (me = traits->type)) {
		if (ptr) *((const void **) ptr) = &od->_in;
		return MPT_ENUM(TypeUnixSocket);
	}
	if (!type) {
		static const uint8_t fmt[] = {
			MPT_ENUM(TypeObjectPtr),
			MPT_ENUM(TypeOutputPtr),
			MPT_ENUM(TypeLoggerPtr),
			0
		};
		if (ptr) {
			*((const uint8_t **) ptr) = fmt;
		}
		return me;
	}
	if (type == MPT_ENUM(TypeMetaPtr)) {
		if (ptr) *((const void **) ptr) = &od->_in;
		return MPT_ENUM(TypeUnixSocket);
	}
	if (type == MPT_ENUM(TypeUnixSocket)) {
		if (ptr) *((int *) ptr) = remote_infile(od);
		return me;
	}
	if (type == MPT_ENUM(TypeObjectPtr)) {
		if (ptr) *((const void **) ptr) = &od->_obj;
		return me;
	}
	if (type == MPT_ENUM(TypeOutputPtr)) {
		if (ptr) *((const void **) ptr) = &od->_out;
		return me;
	}
	if (type == MPT_ENUM(TypeLoggerPtr)) {
		if (ptr) *((const void **) ptr) = &od->_log;
		return me;
	}
	return MPT_ERROR(BadType);
}
/* metatype interface */
static void remoteUnref(MPT_INTERFACE(metatype) *mt)
{
	MPT_STRUCT(out_data) *od = MPT_baseaddr(out_data, mt, _in);
	uintptr_t c;
	if ((c = mpt_refcount_lower(&od->_ref))) {
		return;
	}
	mpt_connection_fini(&od->con);
	free(od);
}
static uintptr_t remoteRef(MPT_INTERFACE(metatype) *mt)
{
	MPT_STRUCT(out_data) *od = MPT_baseaddr(out_data, mt, _in);
	
	return mpt_refcount_raise(&od->_ref);
}
static MPT_INTERFACE(metatype) *remoteClone(const MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
/* input interface */
static int remoteNext(MPT_INTERFACE(input) *in, int what)
{
	MPT_STRUCT(out_data) *od = MPT_baseaddr(out_data, in, _in);
	MPT_STRUCT(buffer) *buf;
	int keep = 0;
	
	if (!MPT_socket_active(&od->con.out.sock)) {
		if (!(buf = od->con.out.buf._buf)) {
			return -3;
		}
		return mpt_stream_poll((void *) buf, what, 0);
	}
	if (what & POLLIN) {
		int ret;
		/* existing input data */
		if ((od->con.out.state & MPT_OUTFLAG(Received))) {
			keep = POLLIN;
		}
		/* get new datagram */
		else if ((ret = mpt_outdata_recv(&od->con.out)) < 0) {
			mpt_log(0, __func__, MPT_LOG(Error), "%s: %s",
			        MPT_tr("receive failed"), MPT_tr("unable to get new data"));
		}
		/* message size invalid */
		else if (ret < od->con.out._idlen) {
			mpt_log(0, __func__, MPT_LOG(Error), "%s: %s: %d < %d",
			        MPT_tr("bad message size"), MPT_tr("messag smaller than id"),
			        ret, od->con.out._idlen);
		}
		/* save input parameters */
		else {
			keep = POLLIN;
			what &= ~POLLHUP;
		}
	}
	if ((what & POLLOUT)
	    && !(od->con.out.state & (MPT_OUTFLAG(Active) | MPT_OUTFLAG(Received)))
	    && (buf = od->con.out.buf._buf)
	    && buf->_used) {
		const struct sockaddr *addr = 0;
		uint8_t *base = (void *) (buf + 1);
		ssize_t len = od->con.out._scurr;
		
		if (len) {
			addr = (const struct sockaddr *) (base + buf->_used - len);
		}
		if (sendto(od->con.out.sock._id, base, buf->_used, 0, addr, len) >= 0) {
			buf->_used = 0;
			od->con.out._scurr = 0;
			if (keep < 0) {
				keep = 0;
			}
		}
	}
	if (what & POLLHUP) {
		mpt_outdata_close(&od->con.out);
		return -2;
	}
	/* dispatch dereference to notifier */
	return keep;
}
static int remoteDispatch(MPT_INTERFACE(input) *in, MPT_TYPE(event_handler) cmd, void *arg)
{
	MPT_STRUCT(out_data) *od = MPT_baseaddr(out_data, in, _in);
	return mpt_connection_dispatch(&od->con, cmd, arg);
}
/* object interface */
static int remoteProperty(const MPT_STRUCT(object) *obj, MPT_STRUCT(property) *pr)
{
	MPT_STRUCT(out_data) *od = MPT_baseaddr(out_data, obj, _obj);
	
	if (!pr) {
		return MPT_ENUM(TypeOutputPtr);
	}
	if (pr->name && !*pr->name) {
		pr->name = "output";
		pr->desc = "generic output interface";
		MPT_value_set_data(&pr->val, MPT_ENUM(TypeUnixSocket), &od->con.out.sock);
		
		return MPT_socket_active(&od->con.out.sock) ? 1 : 0;
	}
	return mpt_connection_get(&od->con, pr);
}
static int remoteSetProperty(MPT_INTERFACE(object) *obj, const char *name, MPT_INTERFACE(convertable) *src) {
	static const char _fcn[] = "mpt::output::setProperty";
	
	MPT_STRUCT(out_data) *od = MPT_baseaddr(out_data, obj, _obj);
	int ret;
	
	ret = mpt_connection_set(&od->con, name, src);
	if (ret < 0) {
		if (!name || !*name) {
			mpt_log(0, _fcn, MPT_LOG(Debug), "%s",
			        MPT_tr("unable to assign output"));
		} else {
			mpt_log(0, _fcn, MPT_LOG(Debug), "%s: %s",
			        MPT_tr("unable to set property"), name);
		}
	}
	return ret;
}
/* output interface */
static ssize_t remotePush(MPT_INTERFACE(output) *out, size_t len, const void *src)
{
	MPT_STRUCT(out_data) *od = MPT_baseaddr(out_data, out, _out);
	return mpt_connection_push(&od->con, len, src);
}
static int remoteSync(MPT_INTERFACE(output) *out, int timeout)
{
	static const char _func[] = "mpt::output::sync";
	
	MPT_STRUCT(out_data) *od = MPT_baseaddr(out_data, out, _out);
	int pos;
	
	if (!od->con.out._idlen) {
		return 0;
	}
	if (!MPT_socket_active(&od->con.out.sock)) {
		MPT_STRUCT(stream) *srm;
		
		if (!(srm = (void *) od->con.out.buf._buf)) {
			return MPT_ERROR(BadArgument);
		}
		return mpt_stream_sync(srm, od->con.out._idlen, &od->con._wait, timeout);
	}
	while (1) {
		MPT_STRUCT(buffer) *buf;
		MPT_STRUCT(command) *ans;
		uint64_t ansid;
		uint8_t *data, idlen, smax;
		
		/* use existing data */
		if (!(od->con.out.state & MPT_OUTFLAG(Received))) {
			struct pollfd p;
			
			p.fd = od->con.out.sock._id;
			p.events = POLLIN;
			
			if ((pos = poll(&p, 1, timeout)) < 0) {
				return MPT_ERROR(BadOperation);
			}
			if (!pos) {
				return 0;
			}
			if ((pos = mpt_outdata_recv(&od->con.out)) < 0) {
				mpt_log(0, __func__, MPT_LOG(Error), "%s: %s",
				        MPT_tr("receive failed"), MPT_tr("unable to get new data"));
				return MPT_ERROR(BadOperation);
			}
			timeout = 0;
		}
		idlen = od->con.out._idlen;
		smax = od->con.out._smax;
		buf = od->con.out.buf._buf;
		data = (void *) (buf + 1);
		
		if (!buf || buf->_used < (idlen + smax)) {
			mpt_log(0, __func__, MPT_LOG(Error), "%s: %s",
			        MPT_tr("bad message size"), MPT_tr("unable to get new data"));
			return MPT_ERROR(BadValue);
		}
		data += smax;
		/* no reply message */
		if (!(data[0] & 0x80)) {
			size_t max, len;
			
			if (!(buf = od->con._wait._buf)) {
				return 0;
			}
			ans = (void *) (buf + 1);
			max = buf->_used / sizeof(*ans);
			len = 0;
			
			/* count waiting slots */
			while (max--) {
				if ((ans++)->cmd) {
					++len;
				}
			}
			return len;
		}
		od->con.out.state &= ~MPT_OUTFLAG(Received);
		data[0] &= 0x7f;
		pos = mpt_message_buf2id(data, idlen, &ansid);
		
		if (pos < 0 || pos > (int) sizeof(uintptr_t)) {
			mpt_log(0, _func, MPT_LOG(Error), "%s (%i)",
			        MPT_tr("bad message id"), pos);
			return MPT_ERROR(BadValue);
		}
		if ((ans = mpt_command_get(&od->con._wait, ansid))) {
			MPT_STRUCT(message) msg;
			
			msg.base = data + idlen;
			msg.used = buf->_used - idlen;
			msg.cont = 0;
			msg.clen = 0;
			
			if (ans->cmd(ans->arg, &msg) < 0) {
				return 0;
			}
			continue;
		}
		mpt_log(0, _func, MPT_LOG(Error), "%s (%" PRIx64 ")",
		        MPT_tr("bad reply id"), ansid);
		return MPT_ERROR(BadValue);
	}
}
static int remoteAwait(MPT_INTERFACE(output) *out, int (*ctl)(void *, const MPT_STRUCT(message) *), void *udata)
{
	MPT_STRUCT(out_data) *od = MPT_baseaddr(out_data, out, _out);
	return mpt_connection_await(&od->con, ctl, udata);
}
/* logger interface */
static int remoteLog(MPT_INTERFACE(logger) *log, const char *from, int type, const char *fmt, va_list arg)
{
	MPT_INTERFACE(output) *out = &MPT_baseaddr(out_data, log, _log)->_out;
	return mpt_output_vlog(out, from, type, fmt, arg);
}

/*!
 * \ingroup mptOutput
 * \brief create output
 * 
 * Create new output metatype to be connected to
 * remote or local socket types.
 * Can also write to and read from coded local files.
 * 
 * \return remote output metatype
 */
extern MPT_INTERFACE(input) *mpt_output_remote()
{
	static const MPT_INTERFACE_VPTR(input) remoteInput = {
		{ { remoteConv },
		  remoteUnref,
		  remoteRef,
		  remoteClone
		},
		remoteNext,
		remoteDispatch
	};
	static const MPT_INTERFACE_VPTR(object) remoteObject = {
		remoteProperty,
		remoteSetProperty
	};
	static const MPT_INTERFACE_VPTR(output) remoteOutput = {
		remotePush,
		remoteSync,
		remoteAwait
	};
	static const MPT_INTERFACE_VPTR(logger) remoteLogger = {
		remoteLog
	};
	static const MPT_STRUCT(out_data) defOut = {
		{ &remoteInput }, { &remoteObject }, { &remoteOutput }, { &remoteLogger },
		{ 1 },
		MPT_CONNECTION_INIT
	};
	MPT_STRUCT(out_data) *od;
	
	if (!(od = malloc(sizeof(*od)))) {
		return 0;
	}
	*od = defOut;
	
	od->con.out.state = MPT_OUTFLAG(PrintColor);
	
	return &od->_in;
}
