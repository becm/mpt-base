/*!
 * set metatype data.
 */

#include <stdarg.h>
#include <string.h>

#include "types.h"
#include "convert.h"

#include "meta.h"

struct iteratorVararg
{
	MPT_INTERFACE(iterator) _it;
	const char *fmt;
	va_list arg;
	struct {
		const char *fmt;
		va_list arg;
	} org;
	MPT_STRUCT(value) val;
};

static const MPT_STRUCT(value) *iteratorVarargValue(MPT_INTERFACE(iterator) *it)
{
	const struct iteratorVararg *va = (void *) it;
	return &va->val;
}
static int iteratorVarargAdvance(MPT_INTERFACE(iterator) *it)
{
	struct iteratorVararg *va = (void *) it;
	int ret;
	if (!va->fmt) {
		return MPT_ERROR(MissingData);
	}
	if (!(ret = *va->fmt)) {
		va->val.type = 0;
		return 0;
	}
	if ((ret = mpt_value_argv(&va->val, ret, va->arg)) < 0) {
		return ret;
	}
	++va->fmt;
	return va->val.type;
}
static int iteratorVarargReset(MPT_INTERFACE(iterator) *it)
{
	struct iteratorVararg *va = (void *) it;
	const char *fmt = va->org.fmt;
	if (fmt && *fmt) {
		va_copy(va->arg, va->org.arg);
	}
	va->fmt = fmt;
	return strlen(va->fmt);
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
		iteratorVarargValue,
		iteratorVarargAdvance,
		iteratorVarargReset
	};
	struct iteratorVararg va;
	int ret;
	
	if (!proc) {
		return MPT_ERROR(BadArgument);
	}
	va._it._vptr = &ctl;
	va.val.type = 0;
	*((uint8_t *) &va.val._bufsize) = sizeof(va.val._buf);
	if (!(va.fmt = fmt) || !(ret = *fmt)) {
		return proc(ctx, &va._it);
	}
	va_copy(va.arg, arg);
	va_copy(va.org.arg, arg);
	if ((ret = mpt_value_argv(&va.val, ret, va.arg)) >= 0) {
		++va.fmt;
		ret = proc(ctx, &va._it);
	}
	va_end(va.arg);
	return ret;
}
