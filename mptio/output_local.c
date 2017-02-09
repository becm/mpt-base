/*!
 * finalize connection data
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "event.h"
#include "message.h"
#include "meta.h"
#include "array.h"

#include "stream.h"
#include "convert.h"

#include "output.h"

#define AnswerFlags(a) ((a & 0xf0) >> 4)
#define OutputFlags(a) (a & 0xf)

MPT_STRUCT(local_output)
{
	MPT_INTERFACE(output) _out;
	
	MPT_STRUCT(reference) ref;
	
	MPT_INTERFACE(output) *pass;
	MPT_STRUCT(history) hist;
};

static int setHistfile(FILE **hist, MPT_INTERFACE(metatype) *src)
{
	const char *where = 0;
	int len;
	FILE *fd;
	
	if (src && (len = src->_vptr->conv(src, 's', &where)) < 0) {
		return len;
	}
	
	if (!where) {
		fd = stdout;
	} else if (!*where) {
		fd = 0;
	} else {
		MPT_STRUCT(socket) sock = MPT_SOCKET_INIT;
		int mode;
		
		/* try to use argument as connect string */
		if ((mode = mpt_connect(&sock, where, 0)) >= 0) {
			if (!(mode & MPT_SOCKETFLAG(Stream))
			    || !(mode & MPT_SOCKETFLAG(Write))
			    || !(fd = fdopen(sock._id, "w"))) {
				mpt_connect(&sock, 0, 0);
				return -1;
			}
		}
		/* regular file path */
		else if (!(fd = fopen(where, "w"))) {
			return -1;
		}
	}
	if (*hist && (*hist != stdout) && (*hist != stderr)) {
		fclose(*hist);
	}
	*hist = fd;
	
	return len;
}
/* output operations */
static ssize_t localPush(MPT_INTERFACE(output) *out, size_t len, const void *src)
{
	MPT_STRUCT(local_output) *lo = (void *) out;
	int ret;
	
	out = lo->pass;
	
	/* remote output active */
	if (lo->hist.info.state & MPT_OUTFLAG(Remote)) {
		if (!out) {
			return MPT_ERROR(BadArgument);
		}
		ret = out->_vptr->push(out, len, src);
		if (ret >= 0 && !len) {
			lo->hist.info.state &= ~MPT_OUTFLAG(Remote);
		}
		return ret;
	}
	/* try local output */
	ret = mpt_history_push(&lo->hist, len, src);
	
	/* invalid local output operation */
	if (ret < 0 && !(lo->hist.info.state & MPT_OUTFLAG(Active))) {
		if (!out || (ret = out->_vptr->push(out, len, src)) < 0) {
			return ret;
		}
		if (len && src) {
			lo->hist.info.state |= MPT_OUTFLAG(Remote);
		}
	}
	return ret;
}
static int localAwait(MPT_INTERFACE(output) *out, int (*ctl)(void *, const MPT_STRUCT(message) *), void *udata)
{
	MPT_STRUCT(local_output) *lo = (void *) out;
	int ret;
	if (!(out = lo->pass)) {
		return MPT_ERROR(BadOperation);
	}
	ret = out->_vptr->await(out, ctl, udata);
	
	if (ret >= 0) {
		lo->hist.info.state |= MPT_OUTFLAG(Remote);
	}
	return ret;
}
static int localSync(MPT_INTERFACE(output) *out, int timeout)
{
	MPT_STRUCT(local_output) *lo = (void *) out;
	out = lo->pass;
	return out ? out->_vptr->sync(out, timeout) : 0;
}
/* object property handlers */
static int localSet(MPT_INTERFACE(object) *out, const char *name, MPT_INTERFACE(metatype) *src)
{
	MPT_STRUCT(local_output) *lo = (void *) out;
	
	out = (void *) lo->pass;
	
	if (!name) {
		return setHistfile(&lo->hist.info.file, src);
	}
	if (!*name) {
		MPT_INTERFACE(output) *no;
		int ret;
		if (!src) {
			return MPT_ERROR(BadValue);
		}
		if ((ret = src->_vptr->conv(src, MPT_ENUM(TypeOutput), &no)) < 0) {
			return ret;
		}
		if (no && !no->_vptr->obj.addref((void *) no)) {
			return MPT_ERROR(BadOperation);
		}
		if (out) out->_vptr->ref.unref((void *) out);
		lo->pass = no;
		return ret;
	}
	if (!strcasecmp(name, "file")) {
		return setHistfile(&lo->hist.info.file, src);
	}
	if (!strcasecmp(name, "format") || !strcasecmp(name, "fmt")) {
		return mpt_valfmt_set(&lo->hist.fmt._fmt, src);
	}
	if ((out = (void *) lo->pass)) {
		return out->_vptr->setProperty(out, name, src);
	}
	return MPT_ERROR(BadArgument);
}
static int localGet(const MPT_INTERFACE(object) *out, MPT_STRUCT(property) *pr)
{
	MPT_STRUCT(local_output) *lo = (void *) out;
	const char *name;
	intptr_t pos = -1, id;
	
	if (!pr) {
		return MPT_ENUM(TypeOutput);
	}
	if (!(name = pr->name)) {
		pos = (intptr_t) pr->desc;
	}
	id = -1;
	if (name ? !*name : pos == ++id) {
		static const char fmt[] = { MPT_ENUM(TypeOutput), 0 };
		pr->name = "output";
		pr->desc = MPT_tr("chained output descriptor");
		pr->val.fmt = fmt;
		pr->val.ptr = &lo->pass;
		return lo->pass ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "history") || !strcasecmp(name, "histfile") || !strcasecmp(name, "file")) : pos == ++id) {
		static const char fmt[] = { MPT_ENUM(TypeFile), 0 };
		pr->name = "file";
		pr->desc = MPT_tr("history data output file");
		pr->val.fmt = fmt;
		pr->val.ptr = &lo->hist.info.file;
		return lo->hist.info.file ? 1 : 0;
	}
	if (name ? (!strcasecmp(name, "histfmt") || !strcasecmp(name, "format") || !strcasecmp(name, "fmt")) :  pos == ++id) {
		static const char fmt[] = { MPT_ENUM(TypeValFmt), 0 };
		MPT_STRUCT(buffer) *buf;
		pr->name = "format";
		pr->desc = MPT_tr("history data output format");
		pr->val.fmt = fmt;
		pr->val.ptr = 0;
		if (!(buf = lo->hist.fmt._fmt._buf)) {
			return 0;
		}
		pr->val.ptr = buf + 1;
		return buf->used / sizeof(MPT_STRUCT(valfmt));
	}
	return MPT_ERROR(BadArgument);
}
/* reference operations */
uintptr_t localRef(MPT_INTERFACE(object) *obj)
{
	MPT_STRUCT(local_output) *lo = (void *) obj;
	return mpt_reference_raise(&lo->ref);
}
static void localUnref(MPT_INTERFACE(unrefable) *ref)
{
	MPT_STRUCT(local_output) *lo = (void *) ref;
	
	/* remove active reference */
	if (mpt_reference_lower(&lo->ref)) {
		return;
	}
	mpt_history_fini(&lo->hist);
	
	if ((ref = (void *) lo->pass)) {
		ref->_vptr->unref(ref);
	}
	free(lo);
}

static const MPT_INTERFACE_VPTR(output) localCtl = {
	{ { localUnref }, localRef, localGet, localSet },
	localPush,
	localSync,
	localAwait
};

/*!
 * \ingroup mptOutput
 * \brief create output
 * 
 * New output descriptor.
 * 
 * If existing output descriptor is supplied
 * incompatible or explicit remote messages
 * are redirected.
 * 
 * Create output takes ownership of passed reference.
 * 
 * \param pass  chained output descriptor
 * 
 * \return output descriptor
 */
extern MPT_INTERFACE(output) *mpt_output_local(MPT_INTERFACE(output) *pass)
{
	static const MPT_STRUCT(local_output) defOut = {
		{ &localCtl }, { 1 }, 0, MPT_HISTORY_INIT
	};
	MPT_STRUCT(local_output) *od;
	
	if (!(od = malloc(sizeof(*od)))) {
		return 0;
	}
	*od = defOut;
	
	setHistfile(&od->hist.info.file, 0);
	
	od->pass = pass;
	
	od->hist.info.state  = MPT_OUTFLAG(PrintColor);
	od->hist.info.ignore = MPT_LOG(Info);
	
	return &od->_out;
}

