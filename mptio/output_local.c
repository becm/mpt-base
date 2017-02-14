/*!
 * finalize connection data
 */

#include <stdio.h>
#include <stdlib.h>

#include "meta.h"

#include "output.h"

MPT_STRUCT(local_output)
{
	MPT_INTERFACE(output) _out;
	
	MPT_STRUCT(reference) ref;
	
	MPT_INTERFACE(output) *pass;
	MPT_STRUCT(history) hist;
};
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
	int ret;
	
	out = (void *) lo->pass;
	
	if (!name) {
		return mpt_history_set(&lo->hist, name, src);
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
	if ((ret = mpt_history_set(&lo->hist, name, src)) >= 0) {
		return ret;
	}
	return MPT_ERROR(BadArgument);
}
static int localGet(const MPT_INTERFACE(object) *out, MPT_STRUCT(property) *pr)
{
	MPT_STRUCT(local_output) *lo = (void *) out;
	const char *name;
	
	if (!pr) {
		return MPT_ENUM(TypeOutput);
	}
	if ((name = pr->name) && !*name) {
		static const char fmt[] = { MPT_ENUM(TypeOutput), 0 };
		pr->name = "history";
		pr->desc = MPT_tr("local output filter");
		pr->val.fmt = fmt;
		pr->val.ptr = &lo->pass;
		return lo->pass ? 1 : 0;
	}
	return mpt_history_get(&lo->hist, pr);
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
 * New local output descriptor.
 * 
 * If existing output descriptor is supplied
 * incompatible or explicit remote messages
 * are redirected.
 * 
 * Created instance takes ownership of passed reference.
 * 
 * \param pass  chained output descriptor
 * 
 * \return output descriptor
 */
extern MPT_INTERFACE(output) *mpt_output_local(void)
{
	static const MPT_STRUCT(local_output) defOut = {
		{ &localCtl }, { 1 }, 0, MPT_HISTORY_INIT
	};
	MPT_STRUCT(local_output) *od;
	
	if (!(od = malloc(sizeof(*od)))) {
		return 0;
	}
	*od = defOut;
	
	od->hist.info.file = stdout;
	od->hist.info.state  = MPT_OUTFLAG(PrintColor);
	od->hist.info.ignore = MPT_LOG(Info);
	
	return &od->_out;
}

