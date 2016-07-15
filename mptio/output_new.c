/*!
 * set initial parameter for output descriptor
 */

#ifndef _POSIX_C_SOURCE
# define _POSIX_C_SOURCE 200809L
#endif

#include <stdlib.h>
#include <inttypes.h>

#include <sys/uio.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <unistd.h>
#include <poll.h>

#include "meta.h"
#include "message.h"
#include "output.h"

#include "stream.h"
#include "notify.h"

MPT_STRUCT(out_data) {
	MPT_INTERFACE(metatype) _mt;
	MPT_INTERFACE(output)   _out;
	MPT_INTERFACE(input)    _in;
	
	MPT_STRUCT(reference) _ref;
	MPT_STRUCT(notify)   *_no;
	
	MPT_STRUCT(connection) con;
};

/* metatype interface */
static void metaUnref(MPT_INTERFACE(unrefable) *mt)
{
	MPT_STRUCT(out_data) *od = (void *) mt;
	uintptr_t c;
	if ((c = mpt_reference_lower(&od->_ref))) {
		return;
	}
	mpt_connection_fini(&od->con);
	free(od);
}
static int metaAssign(MPT_INTERFACE(metatype) *mt, const MPT_STRUCT(value) *val)
{
	MPT_STRUCT(out_data) *od = (void *) mt;
	if (!val) {
		return mpt_connection_set(&od->con, 0, 0);
	} else {
		return mpt_object_pset((void *) &od->_out, 0, val, 0);
	}
}
static int metaConv(MPT_INTERFACE(metatype) *mt, int type, void *addr)
{
	static const char fmt[] = { MPT_ENUM(TypeMeta), MPT_ENUM(TypeOutput), MPT_ENUM(TypeInput), 0 };
	MPT_STRUCT(out_data) *od = (void *) mt;
	void *ptr;
	switch (type) {
	  case 0: ptr = (void *) fmt; type = MPT_ENUM(TypeOutput); break;
	  case MPT_ENUM(TypeMeta): ptr = &od->_mt; break;
	  case MPT_ENUM(TypeOutput): ptr = &od->_out; break;
	  case MPT_ENUM(TypeInput): ptr = &od->_in; break;
	  default: return MPT_ERROR(BadType);
	}
	if (addr) {
		*((void **) addr) = ptr;
	}
	return type;
}
static MPT_INTERFACE(metatype) *metaClone(const MPT_INTERFACE(metatype) *from)
{
	(void) from;
	return 0;
}
static const MPT_INTERFACE_VPTR(metatype) metaCtl = {
	{ metaUnref },
	metaAssign,
	metaConv,
	metaClone
};
/* object interface */
static void outputUnref(MPT_INTERFACE(unrefable) *obj)
{
	MPT_STRUCT(out_data) *od = MPT_reladdr(out_data, obj, _out, _mt);
	uintptr_t c;
	if ((c = mpt_reference_lower(&od->_ref))) {
		return;
	}
	mpt_connection_fini(&od->con);
	free(od);
}
static uintptr_t outputRef(MPT_INTERFACE(object) *obj)
{
	MPT_STRUCT(out_data) *od = MPT_reladdr(out_data, obj, _out, _mt);
	
	return mpt_reference_raise(&od->_ref);
}
static int outputProperty(const MPT_STRUCT(object) *obj, MPT_STRUCT(property) *pr)
{
	MPT_STRUCT(out_data) *od = MPT_reladdr(out_data, obj, _out, _mt);
	
	if (!pr) {
		return MPT_ENUM(TypeOutput);
	}
	if (pr->name && !*pr->name) {
		static const char sock[] = { MPT_ENUM(TypeSocket), 0 };
		pr->name = "output";
		pr->desc = "generic output interface";
		pr->val.fmt = sock;
		pr->val.ptr = &od->con.out.sock;
		
		return 0;
	}
	return mpt_connection_get(&od->con, pr);
}
static int outputSetProperty(MPT_INTERFACE(object) *obj, const char *name, MPT_INTERFACE(metatype) *src) {
	static const char _fcn[] = "mpt::output::setProperty";
	
	MPT_STRUCT(out_data) *od = MPT_reladdr(out_data, obj, _out, _mt);
	MPT_INTERFACE(input) *in = &od->_in;
	int ret, oldFd, newFd;
	
	oldFd = in->_vptr->_file(in);
	
	ret = mpt_connection_set(&od->con, name, src);
	if (ret < 0) {
		if (!name || !*name) {
			mpt_output_log(&od->_out, _fcn, MPT_LOG(Debug), "%s",
			               MPT_tr("unable to assign output"));
		} else {
			mpt_output_log(&od->_out, _fcn, MPT_LOG(Debug), "%s: %s",
			               MPT_tr("unable to set property"), name);
		}
	}
	newFd = in->_vptr->_file(in);
	
	if (!od->_no || oldFd == newFd) {
		return ret;
	}
	/* remove old registration */
	if (oldFd > 2) {
		mpt_notify_clear(od->_no, oldFd);
	}
	if (ret < 0 || newFd < 0) {
		return ret;
	}
	/* add local reference for event controller */
	if (!outputRef((void *) &od->_out)) {
		mpt_output_log(&od->_out, _fcn, MPT_LOG(Error), "%s: %s "PRIxPTR,
		               MPT_tr("failed"),
		               MPT_tr("reference output"),
		               od);
	}
	/* use first reference for notifier */
	else if (mpt_notify_add(od->_no, POLLIN, &od->_in) < 0) {
		mpt_output_log(&od->_out, _fcn, MPT_LOG(Error), "%s: %s: fd%i",
		               MPT_tr("failed"),
		               MPT_tr("register notifier"),
		               newFd);
		/* clear references */
		outputUnref((void *) &od->_out);
	}
	return ret;
}
/* output interface */
static ssize_t outputPush(MPT_INTERFACE(output) *out, size_t len, const void *src)
{
	MPT_STRUCT(out_data) *od = MPT_reladdr(out_data, out, _out, _mt);
	return mpt_connection_push(&od->con, len, src);
}
static int outputSync(MPT_INTERFACE(output) *out, int timeout)
{
	static const char _func[] = "mpt::output::sync";
	MPT_STRUCT(out_data) *od = MPT_reladdr(out_data, out, _out, _mt);
	
	if (!od->con.out._idlen) {
		return 0;
	}
	if (!MPT_socket_active(&od->con.out.sock)) {
		MPT_STRUCT(stream) *srm;
		
		if (!(srm = od->con.out._buf)) {
			return MPT_ERROR(BadArgument);
		}
		return mpt_stream_sync(srm, od->con.out._idlen, &od->con._wait, timeout);
	}
	while (1) {
		MPT_STRUCT(reply_context) *rc;
		MPT_STRUCT(buffer) *buf;
		MPT_STRUCT(command) *ans;
		uint64_t ansid;
		int pos;
		
		/* use existing data */
		if (od->con.out.state & MPT_ENUM(OutputReceived)) {
			if (!(pos = od->con._ctxpos)) {
				return 0;
			}
		}
		/* get new datagram */
		else {
			struct pollfd p;
			
			p.fd = od->con.out.sock._id;
			p.events = POLLIN;
			
			if ((pos = poll(&p, 1, timeout)) < 0) {
				return MPT_ERROR(BadOperation);
			}
			if (!pos) {
				return 0;
			}
			timeout = 0;
			if ((pos = mpt_outdata_recv(&od->con.out)) < 0) {
				mpt_output_log(&od->_out, __func__, MPT_LOG(Error), "%s: %s",
				               MPT_tr("receive failed"), MPT_tr("unable to get new data"));
				return MPT_ERROR(BadOperation);
			}
			if (!(od->con._ctxpos = pos)) {
				return 0;
			}
		}
		/* no context or reply */
		if (!(buf = od->con.out._ctx._buf)
		    || !(rc = ((void **) (buf + 1))[pos-1])
		    || !(rc->_val[0] & 0x80)) {
			return 0;
		}
		od->con.out.state &= ~MPT_ENUM(OutputReceived);
		rc->_val[0] &= 0x7f;
		pos = mpt_message_buf2id(rc->_val, rc->len, &ansid);
		
		if (pos < 0 || pos > (int) sizeof(uintptr_t)) {
			mpt_output_log(&od->_out, _func, MPT_LOG(Error), "%s (%i)",
			               MPT_tr("bad message id"), pos);
			return MPT_ERROR(BadValue);
		}
		if ((ans = mpt_command_get(&od->con._wait, ansid))) {
			MPT_STRUCT(message) msg;
			
			buf = od->con.out._buf;
			msg.base = buf + 1;
			msg.used = buf->used;
			msg.cont = 0;
			msg.clen = 0;
			
			if (ans->cmd(ans->arg, &msg) < 0) {
				return 0;
			}
			continue;
		}
		mpt_output_log(&od->_out, _func, MPT_LOG(Error), "%s (%" PRIx64 ")",
		               MPT_tr("bad reply id"), ansid);
		return MPT_ERROR(BadValue);
	}
}
static int outputAwait(MPT_INTERFACE(output) *out, int (*ctl)(void *, const MPT_STRUCT(message) *), void *udata)
{
	MPT_STRUCT(out_data) *od = MPT_reladdr(out_data, out, _out, _mt);
	return mpt_connection_await(&od->con, ctl, udata);
}
static int outputLog(MPT_INTERFACE(output) *out, const char *from, int type, const char *fmt, va_list va)
{
	MPT_STRUCT(out_data) *od = MPT_reladdr(out_data, out, _out, _mt);
	return mpt_connection_log(&od->con, from, type, fmt, va);
}
static const MPT_INTERFACE_VPTR(output) outCtl = {
	{ { outputUnref }, outputRef, outputProperty, outputSetProperty },
	outputPush,
	outputSync,
	outputAwait,
	outputLog
};
/* input interface */
static void outputInputUnref(MPT_INTERFACE(unrefable) *in)
{
	outputUnref(MPT_reladdr(out_data, in, _in, _out));
}
static int outputInputNext(MPT_INTERFACE(input) *in, int what)
{
	MPT_STRUCT(out_data) *od = MPT_reladdr(out_data, in, _in, _out);
	MPT_STRUCT(stream) *srm;
	MPT_STRUCT(buffer) *buf;
	int keep = 0;
	
	if (!MPT_socket_active(&od->con.out.sock)) {
		if (!(srm = od->con.out._buf)) {
			return -3;
		}
		return mpt_stream_poll(srm, what, 0);
	}
	if (what & POLLIN) {
		int ret;
		/* existing input data */
		if ((od->con.out.state & MPT_ENUM(OutputReceived))) {
			keep = POLLIN;
		}
		/* get new datagram */
		else if ((ret = mpt_outdata_recv(&od->con.out)) < 0) {
			mpt_output_log(&od->_out, __func__, MPT_LOG(Error), "%s: %s",
			               MPT_tr("receive failed"), MPT_tr("unable to get new data"));
		}
		/* save input parameters */
		else {
			od->con._ctxpos = ret;
			od->con.out.state |= MPT_ENUM(OutputReceived);
			keep = POLLIN;
		}
	}
	if ((what & POLLOUT)
	    && !od->con._ctxpos
	    && !(od->con.out.state & (MPT_ENUM(OutputActive) | MPT_ENUM(OutputReceived)))
	    && (buf = od->con.out._buf)
	    && buf->used) {
		if (od->con.out._socklen) {
			keep |= POLLOUT;
		}
		else if (send(od->con.out.sock._id, buf+1, buf->used, 0) >= 0) {
			buf->used = 0;
		}
	}
	if (what & POLLHUP) {
		close(od->con.out.sock._id);
		od->con.out.sock._id = -1;
		
		return -2;
	}
	/* dispatch dereference to notifier */
	return keep;
}

static int outputInputDispatch(MPT_INTERFACE(input) *in, MPT_TYPE(EventHandler) cmd, void *arg)
{
	MPT_STRUCT(out_data) *odata = MPT_reladdr(out_data, in, _in, _out);
	return mpt_connection_dispatch(&odata->con, cmd, arg);
}
static int outputInputFile(MPT_INTERFACE(input) *in)
{
	const MPT_STRUCT(out_data) *odata = MPT_reladdr(out_data, in, _in, _out);
	MPT_STRUCT(stream) *srm;
	
	if (!MPT_socket_active(&odata->con.out.sock) && (srm = odata->con.out._buf)) {
		return _mpt_stream_fread(&srm->_info);
	}
	return odata->con.out.sock._id;
}
const MPT_INTERFACE_VPTR(input) inputCtl = {
	{ outputInputUnref },
	outputInputNext,
	outputInputDispatch,
	outputInputFile
};

/*!
 * \ingroup mptOutput
 * \brief create output
 * 
 * New output descriptor.
 * 
 * If notification descriptor is passed
 * input on notifier is updated on connection change.
 * 
 * Passed notifier must be available for output livetime.
 * 
 * \param no notification descriptor
 * 
 * \return output descriptor
 */
extern MPT_INTERFACE(metatype) *mpt_output_new(MPT_STRUCT(notify) *no)
{
	static const MPT_STRUCT(out_data) defOut = {
		{ &metaCtl }, { &outCtl }, { &inputCtl },
		{ 1 }, 0,
		MPT_CONNECTION_INIT
	};
	MPT_STRUCT(out_data) *od;
	
	if (!(od = malloc(sizeof(*od)))) {
		return 0;
	}
	*od = defOut;
	
	od->con.out.state = MPT_ENUM(OutputPrintColor);
	
	od->con.level = (MPT_LOG(LevelWarning) << 4) | MPT_LOG(LevelWarning);
	
	od->_no = no;
	
	return &od->_mt;
}
