/*!
 * finalize connection data
 */

#include <stdio.h>
#include <stdlib.h>

#include "meta.h"
#include "object.h"

#include "output.h"

MPT_STRUCT(local_output)
{
	MPT_INTERFACE(metatype) _mt;
	
	MPT_INTERFACE(object) _obj;
	MPT_INTERFACE(output) _out;
	MPT_INTERFACE(logger) _log;
	
	MPT_STRUCT(refcount) ref;
	
	MPT_INTERFACE(metatype) *pass;
	MPT_STRUCT(history) hist;
};
inline static MPT_INTERFACE(output) *localPassOutput(const MPT_STRUCT(local_output) *lo)
{
	MPT_INTERFACE(metatype) *mt;
	MPT_INTERFACE(output) *out;
	out = 0;
	if (!(mt = lo->pass)
	    || mt->_vptr->conv(mt, MPT_ENUM(TypeOutput), &out) < 0) {
		return 0;
	}
	return out;
}
/* reference interface */
static void localUnref(MPT_INTERFACE(reference) *ref)
{
	MPT_STRUCT(local_output) *lo = (void *) ref;
	
	/* remove active reference */
	if (mpt_refcount_lower(&lo->ref)) {
		return;
	}
	mpt_history_fini(&lo->hist);
	
	if ((ref = (void *) lo->pass)) {
		ref->_vptr->unref(ref);
	}
	free(lo);
}
uintptr_t localRef(MPT_INTERFACE(reference) *ref)
{
	MPT_STRUCT(local_output) *lo = (void *) ref;
	return mpt_refcount_raise(&lo->ref);
}
/* metatype interface */
static int localConv(const MPT_INTERFACE(metatype) *mt, int type, void *ptr)
{
	MPT_STRUCT(local_output) *lo = (void *) mt;
	
	if (!type) {
		static const uint8_t fmt[] = {
			MPT_ENUM(TypeObject),
			MPT_ENUM(TypeLogger),
			MPT_ENUM(TypeOutput),
			MPT_ENUM(TypeFile),
			0
		};
		if (ptr) {
			*((const uint8_t **) ptr) = fmt;
			return 0;
		}
		return MPT_ENUM(TypeObject);
	}
	if (type == MPT_ENUM(TypeMeta)) {
		if (ptr) *((void **) ptr) = lo->pass;
		return MPT_ENUM(TypeOutput);
	}
	if (type == MPT_ENUM(TypeObject)) {
		if (ptr) *((void **) ptr) = &lo->_obj;
		return MPT_ENUM(TypeOutput);
	}
	if (type == MPT_ENUM(TypeOutput)) {
		if (ptr) *((void **) ptr) = &lo->_out;
		return MPT_ENUM(TypeObject);
	}
	if (type == MPT_ENUM(TypeLogger)) {
		if (ptr) *((void **) ptr) = &lo->_log;
		return MPT_ENUM(TypeObject);
	}
	if (type == MPT_ENUM(TypeFile)) {
		if (ptr) *((void **) ptr) = lo->hist.info.file;
		return MPT_ENUM(TypeOutput);
	}
	return MPT_ERROR(BadType);
}
static MPT_INTERFACE(metatype) *localClone(const MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
/* object interface */
static int localGet(const MPT_INTERFACE(object) *obj, MPT_STRUCT(property) *pr)
{
	MPT_STRUCT(local_output) *lo = MPT_baseaddr(local_output, obj, _obj);
	const char *name;
	
	if (!pr) {
		return MPT_ENUM(TypeOutput);
	}
	if ((name = pr->name) && !*name) {
		static const uint8_t fmt[] = { MPT_ENUM(TypeMeta), 0 };
		pr->name = "history";
		pr->desc = MPT_tr("local data output");
		pr->val.fmt = fmt;
		pr->val.ptr = &lo->pass;
		return lo->pass ? 1 : 0;
	}
	return mpt_history_get(&lo->hist, pr);
}
static int localSet(MPT_INTERFACE(object) *obj, const char *name, const MPT_INTERFACE(metatype) *src)
{
	MPT_STRUCT(local_output) *lo = MPT_baseaddr(local_output, obj, _obj);
	int ret;
	
	if (!name) {
		return mpt_history_set(&lo->hist, name, src);
	}
	if (!*name) {
		MPT_INTERFACE(metatype) *mt, *old;
		MPT_INTERFACE(output) *out;
		int ret;
		if (!src) {
			return MPT_ERROR(BadValue);
		}
		/* get modifyable instance pointer */
		mt = 0;
		if ((ret = src->_vptr->conv(src, MPT_ENUM(TypeMeta), &mt)) < 0) {
			return ret;
		}
		if (!mt) {
			return MPT_ERROR(BadValue);
		}
		out = 0;
		if ((ret = mt->_vptr->conv(mt, MPT_ENUM(TypeOutput), &out)) < 0) {
			return ret;
		}
		/* avoid circular redirection */
		if (!out || out == &lo->_out) {
			return MPT_ERROR(BadValue);
		}
		if (!mt->_vptr->ref.addref((void *) mt)) {
			return MPT_ERROR(BadOperation);
		}
		if ((old = lo->pass)) {
			old->_vptr->ref.unref((void *) old);
		}
		lo->pass = mt;
		return ret;
	}
	if ((ret = mpt_history_set(&lo->hist, name, src)) >= 0) {
		return ret;
	}
	return MPT_ERROR(BadArgument);
}
/* output interface */
static ssize_t localPush(MPT_INTERFACE(output) *out, size_t len, const void *src)
{
	MPT_STRUCT(local_output) *lo = MPT_baseaddr(local_output, out, _out);
	ssize_t ret;
	
	/* remote output active */
	if (lo->hist.info.state & MPT_OUTFLAG(Remote)) {
		if (!(out = localPassOutput(lo))) {
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
		if (!(out = localPassOutput(lo))) {
			return MPT_ERROR(BadType);
		}
		if ((ret = out->_vptr->push(out, len, src)) < 0) {
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
	MPT_STRUCT(local_output) *lo = MPT_baseaddr(local_output, out, _out);
	int ret;
	
	if (!(out = localPassOutput(lo))) {
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
	if (!(out = localPassOutput(MPT_baseaddr(local_output, out, _out)))) {
		return MPT_ERROR(BadOperation);
	}
	return out->_vptr->sync(out, timeout);
}
/* logger interface */
static int localLog(MPT_INTERFACE(logger) *log, const char *from, int type, const char *fmt, va_list arg)
{
	MPT_STRUCT(local_output) *lo = MPT_baseaddr(local_output, log, _log);
	
	if (!(lo->hist.info.state & MPT_OUTFLAG(Active))) {
		return mpt_history_log(&lo->hist.info, from, type, fmt, arg);
	}
	if ((log = mpt_log_default())) {
		return log->_vptr->log(log, from, type, fmt, arg);
	}
	return 0;
}

/*!
 * \ingroup mptPlot
 * \brief create local output
 * 
 * Create instance of object metatype adhering to
 * generic module interface as well as output
 * and logger interfaces.
 * 
 * \return output metatype
 */
extern MPT_INTERFACE(metatype) *mpt_output_local(void)
{
	static const MPT_INTERFACE_VPTR(metatype) localMeta = {
		{ localUnref, localRef },
		localConv,
		localClone
	};
	static const MPT_INTERFACE_VPTR(object) localObject = {
		localGet,
		localSet
	};
	static const MPT_INTERFACE_VPTR(output) localOutput = {
		localPush,
		localSync,
		localAwait
	};
	static const MPT_INTERFACE_VPTR(logger) localLogger = {
		localLog
	};
	static const MPT_STRUCT(local_output) defOut = {
		{ &localMeta },
		{ &localObject }, { &localOutput }, { &localLogger },
		{ 1 }, 0, MPT_HISTORY_INIT
	};
	MPT_STRUCT(local_output) *od;
	
	if (!(od = malloc(sizeof(*od)))) {
		return 0;
	}
	*od = defOut;
	
	od->hist.info.file = stdout;
	od->hist.info.state  = MPT_OUTFLAG(PrintColor);
	od->hist.info.ignore = MPT_LOG(Info);
	
	return &od->_mt;
}
