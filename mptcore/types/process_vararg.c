/*!
 * set metatype data.
 */

#include <stdarg.h>
#include <string.h>

#include <sys/uio.h>

#include "types.h"

struct iteratorVararg
{
	MPT_INTERFACE(iterator) _it;
	const char *fmt;
	va_list arg;
	struct {
		const char *fmt;
		va_list arg;
	} org;
	uint8_t _buf[16];
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
	struct iovec vec;
	int type;
	int ret;
	if (!va->fmt) {
		return MPT_ERROR(MissingData);
	}
	if (!(type = *va->fmt)) {
		return 0;
	}
	vec.iov_len = sizeof(va->_buf);
	vec.iov_base = va->_buf;
	if ((ret = mpt_value_argv(&vec, type, va->arg)) < 0) {
		return ret;
	}
	MPT_value_set(&va->val, type, va->_buf);
	++va->fmt;
	return type;
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
	struct iovec vec;
	int type;
	int ret;
	
	if (!proc) {
		return MPT_ERROR(BadArgument);
	}
	va._it._vptr = &ctl;
	if (!(va.fmt = fmt) || !(type = *fmt)) {
		MPT_value_set(&va.val, 0, 0);
		return proc(ctx, &va._it);
	}
	va_copy(va.arg, arg);
	va_copy(va.org.arg, arg);
	vec.iov_len = sizeof(va._buf);
	vec.iov_base = va._buf;
	if ((ret = mpt_value_argv(&vec, type, va.arg)) >= 0) {
		MPT_value_set(&va.val, type, va._buf);
		++va.fmt;
		ret = proc(ctx, &va._it);
	}
	va_end(va.arg);
	return ret;
}
