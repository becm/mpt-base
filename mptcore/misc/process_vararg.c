/*!
 * set metatype data.
 */

#include <stdarg.h>
#include <inttypes.h>

#include "convert.h"

#include "meta.h"

struct iteratorVararg
{
	MPT_INTERFACE(iterator) _it;
	const char *fmt;
	va_list arg;
	size_t len;
	union {
		int8_t   b;
		uint8_t   y;
		int16_t n;
		uint16_t q;
		int32_t i;
		uint32_t u;
		int64_t x;
		uint64_t t;
		
		long l;
		
		float f;
		double d;
#ifdef _MPT_FLOAT_EXTENDED_H
		long double e;
#endif
		void *p;
	} val;
};

static int nextArgument(struct iteratorVararg *va)
{
	int fmt, len;
	if (!va->fmt) {
		return MPT_ERROR(BadOperation);
	}
	if (!(fmt = *va->fmt)) {
		va->fmt = 0;
		return 0;
	}
	switch (fmt) {
		case 'b': va->val.b = va_arg(va->arg, int  );        len = sizeof(int8_t);   break;
		case 'y': va->val.y = va_arg(va->arg, unsigned int); len = sizeof(uint8_t);  break;
		case 'n': va->val.n = va_arg(va->arg, int);          len = sizeof(int16_t);  break;
		case 'q': va->val.q = va_arg(va->arg, unsigned int); len = sizeof(uint16_t); break;
		case 'i': va->val.i = va_arg(va->arg, int32_t);  len = sizeof(int32_t);  break;
		case 'u': va->val.u = va_arg(va->arg, uint32_t); len = sizeof(uint32_t); break;
		case 'x': va->val.x = va_arg(va->arg, int64_t);  len = sizeof(int64_t);  break;
		case 't': va->val.t = va_arg(va->arg, uint64_t); len = sizeof(uint64_t); break;
		
		case 'l': va->val.l = va_arg(va->arg, long); len = sizeof(long); break;
		
		case 'f': va->val.f = va_arg(va->arg, double); len = sizeof(float);  break;
		case 'd': va->val.d = va_arg(va->arg, double); len = sizeof(double); break;
#ifdef _MPT_FLOAT_EXTENDED_H
		case 'e': va->val.e = va_arg(va->arg, long double); len = sizeof(long double); break;
#endif
		case 's':
		case 'k':
		case 'o': va->val.p = va_arg(va->arg, void *); len = sizeof(void *); break;
		default:
			va->len = 0;
			return MPT_ERROR(BadType);
	}
	va->len = len;
	return fmt;
}
static void iteratorVarargUnref(MPT_INTERFACE(unrefable) *ref)
{
	mpt_log(0, "mpt::iterator::vararg", MPT_LOG(Critical), "%s (%" PRIxPTR ")",
	        MPT_tr("tried to unref temporary iterator"), ref);
}
static int iteratorVarargGet(MPT_INTERFACE(iterator) *it, int type, void *ptr)
{
	struct iteratorVararg *va = (void *) it;
	const void *val;
	int fmt;
	if (!va->fmt || !(fmt = *va->fmt)) {
		return 0;
	}
	val = &va->val;
#ifdef MPT_NO_CONVERT
	if (type == fmt) {
		if (ptr) {
			memcpy(ptr, va->val, va->len);
		}
		return fmt;
	}
	return MPT_ERROR(BadType);
#else
	return mpt_data_convert(&val, *va->fmt, ptr, type);
#endif
}
static int iteratorVarargAdvance(MPT_INTERFACE(iterator) *it)
{
	struct iteratorVararg *va = (void *) it;
	if (!va->fmt || !*va->fmt) {
		va->fmt = 0;
		return MPT_ERROR(BadOperation);
	}
	++va->fmt;
	return nextArgument(va);
}
static int iteratorVarargReset(MPT_INTERFACE(iterator) *it)
{
	(void) it;
	return MPT_ERROR(BadOperation);
}

/*!
 * \ingroup mptMeta
 * \brief dispatch value via iterator
 * 
 * Use temporary iterator instance to access
 * value data.
 * 
 * \param val  data to set
 * \param proc iterator process function
 * \param ctx  context for iterator handler
 * 
 * \return value of assignment operation
 */
extern int mpt_process_vararg(const char *fmt, va_list arg, int (*proc)(void *, MPT_INTERFACE(iterator) *), void *ctx)
{
	static const MPT_INTERFACE_VPTR(iterator) ctl = {
		{ iteratorVarargUnref },
		iteratorVarargGet,
		iteratorVarargAdvance,
		iteratorVarargReset
	};
	struct iteratorVararg va;
	int ret;
	
	if (!proc) {
		return MPT_ERROR(BadArgument);
	}
	va._it._vptr = &ctl;
	va.fmt = fmt;
	va_copy(va.arg, arg);
	va.len = 0;
	if (fmt && (ret = nextArgument(&va)) < 0) {
		return ret;
	}
	va_end(va.arg);
	return proc(ctx, &va._it);
}
