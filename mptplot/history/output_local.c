/*!
 * finalize connection data
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "meta.h"
#include "types.h"

#include "object.h"

#include "history.h"

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
	    || MPT_metatype_convert(mt, MPT_ENUM(TypeOutputPtr), &out) < 0) {
		return 0;
	}
	return out;
}
/* convertable interface */
static int localConv(MPT_INTERFACE(convertable) *val, MPT_TYPE(value) type, void *ptr)
{
	MPT_STRUCT(local_output) *lo = (void *) val;
	
	if (!type) {
		static const uint8_t fmt[] = {
			MPT_ENUM(TypeObjectPtr),
			MPT_ENUM(TypeLoggerPtr),
			MPT_ENUM(TypeOutputPtr),
			MPT_ENUM(TypeFilePtr),
			0
		};
		if (ptr) {
			*((const uint8_t **) ptr) = fmt;
		}
		return MPT_ENUM(TypeMetaPtr);
	}
	if (type == MPT_ENUM(TypeMetaPtr)) {
		if (ptr) *((void **) ptr) = lo->pass;
		return MPT_ENUM(TypeOutputPtr);
	}
	if (type == MPT_ENUM(TypeObjectPtr)) {
		if (ptr) *((void **) ptr) = &lo->_obj;
		return MPT_ENUM(TypeOutputPtr);
	}
	if (type == MPT_ENUM(TypeOutputPtr)) {
		if (ptr) *((void **) ptr) = &lo->_out;
		return MPT_ENUM(TypeObjectPtr);
	}
	if (type == MPT_ENUM(TypeLoggerPtr)) {
		if (ptr) *((void **) ptr) = &lo->_log;
		return MPT_ENUM(TypeObjectPtr);
	}
	if (type == MPT_ENUM(TypeFilePtr)) {
		if (ptr) *((void **) ptr) = lo->hist.info.file;
		return MPT_ENUM(TypeOutputPtr);
	}
	return MPT_ERROR(BadType);
}
/* metatype interface */
static void localUnref(MPT_INTERFACE(metatype) *mt)
{
	MPT_STRUCT(local_output) *lo = (void *) mt;
	
	/* remove active reference */
	if (mpt_refcount_lower(&lo->ref)) {
		return;
	}
	mpt_history_fini(&lo->hist);
	
	if ((mt = (void *) lo->pass)) {
		mt->_vptr->unref(mt);
	}
	free(lo);
}
uintptr_t localRef(MPT_INTERFACE(metatype) *mt)
{
	MPT_STRUCT(local_output) *lo = (void *) mt;
	return mpt_refcount_raise(&lo->ref);
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
		return MPT_ENUM(TypeOutputPtr);
	}
	if ((name = pr->name) && !*name) {
		pr->name = "history";
		pr->desc = MPT_tr("local data output");
		MPT_value_set(&pr->val, MPT_ENUM(TypeConvertablePtr), &lo->pass);
		return lo->pass ? 1 : 0;
	}
	return mpt_history_get(&lo->hist, pr);
}
static int localSet(MPT_INTERFACE(object) *obj, const char *name, MPT_INTERFACE(convertable) *src)
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
		if ((ret = src->_vptr->convert(src, MPT_ENUM(TypeMetaPtr), &mt)) < 0) {
			return ret;
		}
		if (!mt) {
			return MPT_ERROR(BadValue);
		}
		out = 0;
		if ((ret = MPT_metatype_convert(mt, MPT_ENUM(TypeOutputPtr), &out)) < 0) {
			return ret;
		}
		/* avoid circular redirection */
		if (!out || out == &lo->_out) {
			return MPT_ERROR(BadValue);
		}
		if (!mt->_vptr->addref(mt)) {
			return MPT_ERROR(BadOperation);
		}
		if ((old = lo->pass)) {
			old->_vptr->unref(old);
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
		return mpt_logfile_log(&lo->hist.info, from, type, fmt, arg);
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
		{ localConv },
		localUnref,
		localRef,
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
