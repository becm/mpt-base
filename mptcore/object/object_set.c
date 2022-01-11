/*!
 * set object data from arguments.
 */

#include <stdlib.h>

#include "meta.h"
#include "convert.h"

#include "object.h"

struct object_property
{
	MPT_INTERFACE(object) *obj;
	const char *prop;
};
/* set object property from iterator */
static int process_object_args(void *ptr, MPT_INTERFACE(iterator) *it)
{
	const struct object_property *op = ptr;
	return mpt_object_set_iterator(op->obj, op->prop, it);
}
/* set object property to arguments */
static int process_object_format(MPT_INTERFACE(object) *obj, const char *prop, const char *fmt, va_list va)
{
	struct object_property op;
	int ret;
	if (!(ret = fmt[1])) {
		/* single value condition */
		MPT_STRUCT(value) val = MPT_VALUE_INIT(0, 0);
		if ((ret = mpt_value_argv(&val, *fmt, va)) < 0) {
			return ret;
		}
		return mpt_object_set_value(obj, prop, &val);
	}
	op.obj = obj;
	op.prop = prop;
	return mpt_process_vararg(fmt, va, process_object_args, &op);
}
/*!
 * \ingroup mptObject
 * \brief set object property
 * 
 * Set property to data in argument list.
 * 
 * \param obj   object interface descriptor
 * \param prop  name of property to change
 * \param fmt   argument format
 * \param va    argument list
 */
extern int mpt_object_vset(MPT_INTERFACE(object) *obj, const char *prop, const char *fmt, va_list va)
{
	if (!fmt) {
		return obj->_vptr->set_property(obj, prop, 0);
	}
	if (!fmt[0]) {
		return obj->_vptr->set_property(obj, prop, (MPT_INTERFACE(convertable) *) mpt_metatype_default());
	}
	return process_object_format(obj, prop, fmt, va);
}
/*!
 * \ingroup mptObject
 * \brief set object property
 * 
 * Set property to data in arguments according to @fmt.
 * 
 * \param obj   object interface descriptor
 * \param prop  name of property to change
 * \param fmt   argument format
 */
extern int mpt_object_set(MPT_INTERFACE(object) *obj, const char *prop, const char *fmt, ...)
{
	va_list va;
	int ret;
	
	if (!fmt) {
		return obj->_vptr->set_property(obj, prop, 0);
	}
	if (!fmt[0]) {
		return obj->_vptr->set_property(obj, prop, (MPT_INTERFACE(convertable) *) mpt_metatype_default());
	}
	va_start(va, fmt);
	ret = process_object_format(obj, prop, fmt, va);
	va_end(va);
	
	return ret;
}
