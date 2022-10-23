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

static int _iteratorVarargNext(struct iteratorVararg *va)
{
	int type;
	int get;
	if (!va->fmt) {
		return MPT_ERROR(MissingData);
	}
	if (!(type = *va->fmt)) {
		va->fmt = 0;
		return 0;
	}
	/* try pure vararg type */
	if ((get = mpt_value_argv(va->_buf, sizeof(va->_buf), type, va->arg)) < 0) {
		const MPT_STRUCT(type_traits) *traits = mpt_type_traits(type);
		/* require trivial copy type */
		if (!traits || traits->init || traits->fini) {
			return MPT_ERROR(BadType);
		}
		/* treat as integer type of same size */
		if ((get = mpt_type_int(traits->size)) <= 0) {
			return MPT_ERROR(BadType);
		}
		if ((get = mpt_value_argv(va->_buf, sizeof(va->_buf), get, va->arg)) < 0) {
			return get;
		}
	}
	MPT_value_set(&va->val, type, va->_buf);
	++va->fmt;
	return type;
}

static const MPT_STRUCT(value) *iteratorVarargValue(MPT_INTERFACE(iterator) *it)
{
	const struct iteratorVararg *va = (void *) it;
	return va->fmt ? &va->val : 0;
}
static int iteratorVarargAdvance(MPT_INTERFACE(iterator) *it)
{
	struct iteratorVararg *va = (void *) it;
	return _iteratorVarargNext(va);
}
static int iteratorVarargReset(MPT_INTERFACE(iterator) *it)
{
	struct iteratorVararg *va = (void *) it;
	const char *fmt = va->org.fmt;
	if (fmt && *fmt) {
		va_end(va->arg);
		va_copy(va->arg, va->org.arg);
	}
	va->fmt = fmt;
	return fmt ? strlen(fmt) : 0;
}

/*!
 * \ingroup mptTypes
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
	int type;
	int ret;
	
	if (!proc) {
		return MPT_ERROR(BadArgument);
	}
	va._it._vptr = &ctl;
	va.org.fmt = fmt;
	va.fmt = fmt;
	if (!fmt || !(type = *fmt)) {
		MPT_value_set(&va.val, 0, 0);
		return proc(ctx, &va._it);
	}
	va_copy(va.arg, arg);
	va_copy(va.org.arg, arg);
	if ((ret = _iteratorVarargNext(&va)) >= 0) {
		ret = proc(ctx, &va._it);
	}
	va_end(va.org.arg);
	va_end(va.arg);
	return ret;
}
