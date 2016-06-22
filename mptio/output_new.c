/*!
 * set initial parameter for output descriptor
 */

#ifndef _POSIX_C_SOURCE
# define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <inttypes.h>

#include <sys/uio.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <poll.h>

#include "array.h"
#include "queue.h"
#include "event.h"
#include "convert.h"
#include "message.h"

#include "stream.h"
#include "notify.h"

#include "output.h"

#define OutputFlags(a) (a & 0xf)

MPT_STRUCT(out_data) {
	MPT_INTERFACE(output) _out;
	MPT_INTERFACE(input)  _in;
	void                  *conv[2];
	
	MPT_STRUCT(reference) _ref;
	MPT_STRUCT(notify)   *_no;
	
	MPT_STRUCT(connection) con;
};

/* metatype interface */
static void outputUnref(MPT_INTERFACE(object) *obj)
{
	MPT_STRUCT(out_data) *od = (void *) obj;
	uintptr_t c;
	if ((c = mpt_reference_lower(&od->_ref))) {
		return;
	}
	mpt_connection_fini(&od->con);
	free(obj);
}
static uintptr_t outputRef(MPT_INTERFACE(object) *obj)
{
	MPT_STRUCT(out_data) *od = (void *) obj;
	
	return mpt_reference_raise(&od->_ref);
}
static int outputProperty(const MPT_STRUCT(object) *obj, MPT_STRUCT(property) *pr)
{
	MPT_STRUCT(out_data) *odata = (void *) obj;
	
	if (!pr) {
		return MPT_ENUM(TypeOutput);
	}
	if (pr->name && !*pr->name) {
		static const char fmt[] = { MPT_ENUM(TypeOutput), MPT_ENUM(TypeInput), 0 };
		pr->name = "output";
		pr->desc = "generic output interface";
		pr->val.fmt = fmt;
		pr->val.ptr = odata->conv;
		
		return 0;
	}
	return mpt_connection_get(&odata->con, pr);
}
static int outputSetProperty(MPT_INTERFACE(object) *obj, const char *name, MPT_INTERFACE(metatype) *src) {
	static const char _fcn[] = "mpt::output::setProperty";
	MPT_STRUCT(out_data) *odata = (void *) obj;
	MPT_INTERFACE(input) *in = &odata->_in;
	int ret, oldFd, newFd;
	
	oldFd = in->_vptr->_file(in);
	
	ret = mpt_connection_set(&odata->con, name, src);
	if (ret < 0) {
		if (!name || !*name) {
			mpt_output_log(&odata->_out, _fcn, MPT_FCNLOG(Debug), "%s",
			               MPT_tr("unable to assign output"));
		} else {
			mpt_output_log(&odata->_out, _fcn, MPT_FCNLOG(Debug), "%s: %s",
			               MPT_tr("unable to set property"), name);
		}
	}
	
	newFd = in->_vptr->_file(in);
	
	if (!odata->_no || oldFd == newFd) {
		return ret;
	}
	/* remove old registration */
	if (oldFd > 2) {
		mpt_notify_clear(odata->_no, oldFd);
	}
	if (ret < 0 || newFd < 0) {
		return ret;
	}
	/* add local reference for event controller */
	if (!outputRef((void *) odata)) {
		mpt_output_log(&odata->_out, _fcn, MPT_FCNLOG(Error), "%s: %s "PRIxPTR,
		               MPT_tr("failed"),
		               MPT_tr("reference output"),
		               odata);
	}
	/* use first reference for notifier */
	else if (mpt_notify_add(odata->_no, POLLIN, &odata->_in) < 0) {
		mpt_output_log(&odata->_out, _fcn, MPT_FCNLOG(Error), "%s: %s: fd%i",
		               MPT_tr("failed"),
		               MPT_tr("register notifier"),
		               newFd);
		/* clear references */
		outputUnref((void *) odata);
	}
	return ret;
}
/* output interface */
static ssize_t outputPush(MPT_INTERFACE(output) *out, size_t len, const void *src)
{
	MPT_STRUCT(out_data) *od = (void *) out;
	return mpt_connection_push(&od->con, len, src);
}
static int outputSync(MPT_INTERFACE(output) *out, int timeout)
{
	MPT_STRUCT(out_data) *od = (void *) out;
	return mpt_outdata_sync(&od->con.out, &od->con._wait, timeout);
}
static int outputAwait(MPT_INTERFACE(output) *out, int (*ctl)(void *, const MPT_STRUCT(message) *), void *udata)
{
	MPT_STRUCT(out_data) *od = (void *) out;
	return mpt_connection_await(&od->con, ctl, udata);
}
static int outputLog(MPT_INTERFACE(output) *out, const char *from, int type, const char *fmt, va_list va)
{
	MPT_STRUCT(out_data) *od = (void *) out;
	return mpt_connection_log(&od->con, from, type, fmt, va);
}
static const MPT_INTERFACE_VPTR(output) outCtl = {
	{ outputUnref, outputRef, outputProperty, outputSetProperty },
	outputPush,
	outputSync,
	outputAwait,
	outputLog
};

static void outputInputUnref(MPT_INTERFACE(input) *in)
{
	outputUnref(MPT_reladdr(out_data, in, _in, _out));
}
static int outputInputNext(MPT_INTERFACE(input) *in, int what)
{
	MPT_STRUCT(out_data) *odata = MPT_reladdr(out_data, in, _in, _out);
	MPT_STRUCT(stream) *srm;
	MPT_STRUCT(buffer) *buf;
	int keep = 0;
	
	if (!MPT_socket_active(&odata->con.out.sock)) {
		if (!(srm = odata->con.out._buf)) {
			return -3;
		}
		return mpt_stream_poll(srm, what, 0);
	}
	if (what & POLLIN) {
		/* handle datagram in dispatch */
		keep = POLLIN;
		
	}
	if ((what & POLLOUT)
	    && (buf = odata->con.out._buf)
	    && buf->used) {
		keep |= POLLOUT;
	}
	if (what & POLLHUP) {
		close(odata->con.out.sock._id);
		odata->con.out.sock._id = -1;
		
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
	
	if (odata->con.out.sock._id < 0 && (srm = odata->con.out._buf)) {
		return _mpt_stream_fread(&srm->_info);
	}
	return odata->con.out.sock._id;
}
const MPT_INTERFACE_VPTR(input) inputCtl = {
	outputInputUnref,
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
extern MPT_INTERFACE(output) *mpt_output_new(MPT_STRUCT(notify) *no)
{
	static const MPT_STRUCT(out_data) defOut = {
		{ &outCtl }, { &inputCtl }, { 0 },
		{ 1 }, 0,
		MPT_CONNECTION_INIT
	};
	MPT_STRUCT(out_data) *odata;
	
	if (!(odata = malloc(sizeof(*odata)))) {
		return 0;
	}
	*odata = defOut;
	
	odata->con.out.state = MPT_ENUM(OutputPrintColor);
	
	odata->con.level = (MPT_ENUM(LogLevelWarning) << 4) | MPT_ENUM(LogLevelWarning);
	
	odata->_no = no;
	
	odata->conv[0] = &odata->_out;
	odata->conv[1] = &odata->_in;
	
	return &odata->_out;
}
