/*!
 * set metatype data.
 */

#include <stdarg.h>
#include <inttypes.h>

#ifdef MPT_NO_CONVERT
# include <string.h>
#endif

#include "convert.h"

#include "meta.h"

struct iteratorVararg
{
	MPT_INTERFACE(iterator) _it;
	const char *fmt;
	va_list arg;
	MPT_STRUCT(scalar) s;
};

static int iteratorVarargGet(MPT_INTERFACE(iterator) *it, int type, void *ptr)
{
	const struct iteratorVararg *va = (void *) it;
	const void *val = &va->s.val;
	if (!va->s.len) {
		return 0;
	}
#ifdef MPT_NO_CONVERT
	if (va->s.type == fmt) {
		if (ptr) memcpy(ptr, val, va->len);
		return fmt;
	}
	return MPT_ERROR(BadType);
#else
	return mpt_data_convert(&val, va->s.type, ptr, type);
#endif
}
static int iteratorVarargAdvance(MPT_INTERFACE(iterator) *it)
{
	struct iteratorVararg *va = (void *) it;
	int ret;
	if (!va->fmt) {
		return MPT_ERROR(MissingData);
	}
	if (!(ret = *va->fmt)) {
		va->fmt = 0;
		va->s.len = 0;
		return 0;
	}
	if ((ret = mpt_scalar_argv(&va->s, ret, va->arg)) < 0) {
		return ret;
	}
	++va->fmt;
	return va->s.type;
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
	va.s.len = 0;
	if (!(va.fmt = fmt) || !(ret = *fmt)) {
		return proc(ctx, &va._it);
	}
	if ((ret = mpt_scalar_argv(&va.s, ret, va.arg)) < 0) {
		return ret;
	}
	va_copy(va.arg, arg);
	ret = proc(ctx, &va._it);
	va_end(va.arg);
	return ret;
}
