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

#include "notify.h"

#include "output.h"

#define OutputFlags(a) (a & 0xf)

MPT_STRUCT(out_data) {
	MPT_INTERFACE(output) _base;
	MPT_INTERFACE(logger) _log;
	MPT_INTERFACE(input)  _in;
	void                  *conv[3];
	
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
		static const char fmt[] = { MPT_ENUM(TypeOutput), MPT_ENUM(TypeLogger), MPT_ENUM(TypeInput), 0 };
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
	int ret, oldFd = odata->con.out.sock._id;
	
	ret = mpt_connection_set(&odata->con, name, src);
	
	if (ret < 0) {
		if (!name || !*name) {
			mpt_log(&odata->_log, _fcn, MPT_FCNLOG(Debug), "%s",
			        MPT_tr("unable to assign output"));
		} else {
			mpt_log(&odata->_log, _fcn, MPT_FCNLOG(Debug), "%s: %s",
			        MPT_tr("unable to set property"), name);
		}
	}
	if (oldFd != odata->con.out.sock._id) {
		MPT_STRUCT(outdata) *od = &odata->con.out;
		MPT_TYPE(DataEncoder) enc = od->_enc.fcn;
		MPT_TYPE(DataDecoder) dec = odata->con.in.dec;
		
		if (od->_sflg & MPT_ENUM(SocketStream)) {
			if (!odata->con._coding) {
				odata->con._coding = MPT_ENUM(EncodingCobs);
				enc = mpt_message_encoder(odata->con._coding);
				dec = mpt_message_decoder(odata->con._coding);
			}
		}
		/* conditions for notification change */
		if (od->_enc.fcn) {
			od->_enc.fcn(&od->_enc.info, 0, 0);
			od->_enc.fcn = enc;
		}
		if (odata->con.in.dec) {
			odata->con.in.dec(&odata->con.in.info, 0, 0);
			odata->con.in.dec = dec;
		}
		if (!odata->_no || (od->sock._id == oldFd)) {
			return ret;
		}
		/* remove old registration */
		if (oldFd > 2) {
			mpt_notify_clear(odata->_no, oldFd);
		}
		if (!MPT_socket_active(&od->sock)) {
			return ret;
		}
		
		/* add local reference for event controller */
		if (!outputRef((void *) odata)) {
			mpt_log(&odata->_log, _fcn, MPT_FCNLOG(Error), "%s: %s "PRIxPTR,
			        MPT_tr("failed"),
			        MPT_tr("reference output"),
			        od);
		}
		/* use first reference for notifier */
		else if (mpt_notify_add(odata->_no, POLLIN, &odata->_in) < 0) {
			mpt_log(&odata->_log, _fcn, MPT_FCNLOG(Error), "%s: %s: fd%i",
			        MPT_tr("failed"),
			        MPT_tr("register notifier"),
			        (int) od->sock._id);
			/* clear references */
			outputUnref((void *) odata);
		}
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
	return mpt_connection_sync(&od->con, timeout);
}
static int outputAwait(MPT_INTERFACE(output) *out, int (*ctl)(void *, const MPT_STRUCT(message) *), void *udata)
{
	MPT_STRUCT(out_data) *od = (void *) out;
	return mpt_connection_await(&od->con, ctl, udata);
}
static const MPT_INTERFACE_VPTR(output) outCtl = {
	{ outputUnref, outputRef, outputProperty, outputSetProperty },
	outputPush,
	outputSync,
	outputAwait
};

/* logger interface */
static void outputLoggerUnref(MPT_INTERFACE(logger) *log)
{
	outputUnref(MPT_reladdr(out_data, log, _log, _base));
}
static int outputLog(MPT_INTERFACE(logger) *log, const char *from, int type, const char *fmt, va_list va)
{
	MPT_STRUCT(out_data) *odata = MPT_reladdr(out_data, log, _log, _base);
	return mpt_connection_log(&odata->con, from, type, fmt, va);
}
static const MPT_INTERFACE_VPTR(logger) logCtl = {
	outputLoggerUnref,
	outputLog
};

static void outputInputUnref(MPT_INTERFACE(input) *in)
{
	outputUnref(MPT_reladdr(out_data, in, _in, _base));
}
static int outputNext(MPT_INTERFACE(input) *in, int what)
{
	MPT_STRUCT(out_data) *odata = MPT_reladdr(out_data, in, _in, _base);
	
	if (!MPT_socket_active(&odata->con.out.sock)) {
		return -3;
	}
	if (what & POLLIN) {
		MPT_STRUCT(queue) *in = &odata->con.in.data;
		ssize_t len;
		
		if (in->len >= in->max && !mpt_queue_prepare(in, 64)) {
			return 0;
		}
		if ((len = mpt_queue_load(in, odata->con.out.sock._id, 0)) > 0) {
			return POLLIN;
		}
		close(odata->con.out.sock._id);
		odata->con.out.sock._id = -1;
		
		if (!in->len) {
			return -3;
		}
	}
	/* dispatch dereference to notifier */
	return (what & POLLHUP) ? -2 : 0;
}

static int outputDispatch(MPT_INTERFACE(input) *in, MPT_TYPE(EventHandler) cmd, void *arg)
{
	MPT_STRUCT(out_data) *odata = MPT_reladdr(out_data, in, _in, _base);
	return mpt_connection_dispatch(&odata->con, cmd, arg);
}
static int outputFile(MPT_INTERFACE(input) *in)
{
	MPT_STRUCT(out_data) *odata = MPT_reladdr(out_data, in, _in, _base);
	return odata->con.out.sock._id;
}
const MPT_INTERFACE_VPTR(input) inputCtl = {
	outputInputUnref,
	
	outputNext,
	outputDispatch,
	outputFile
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
		{ &outCtl }, { &logCtl }, { &inputCtl }, { 0 },
		{ 1 }, 0,
		MPT_CONNECTION_INIT
	};
	MPT_STRUCT(out_data) *odata;
	
	if (!(odata = malloc(sizeof(*odata)))) {
		return 0;
	}
	*odata = defOut;
	
	odata->con.out.state = MPT_ENUM(OutputPrintColor);
	odata->con.out.level = (MPT_ENUM(LogLevelWarning) << 4) | MPT_ENUM(LogLevelWarning);
	
	odata->_no = no;
	
	odata->conv[0] = &odata->_base;
	odata->conv[1] = &odata->_log;
	odata->conv[2] = &odata->_in;
	
	return &odata->_base;
}
