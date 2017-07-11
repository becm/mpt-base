/*!
 * set object data from arguments.
 */

#include "meta.h"

#include "object.h"

struct objectProperty
{
	MPT_INTERFACE(object) *obj;
	const char *prop;
};

static int processObjectArgs(void *ptr, MPT_INTERFACE(iterator) *it)
{
	const struct objectProperty *op = ptr;
	return mpt_object_set_iterator(op->obj, op->prop, it);
}
/*!
 * \ingroup mptObject
 * \brief set object property
 * 
 * Set property to data in argument list.
 * 
 * \param obj    object interface descriptor
 * \param prop   name of property to change
 * \param format argument format
 * \param va     argument list
 */
extern int mpt_object_vset(MPT_INTERFACE(object) *obj, const char *prop, const char *format, va_list va)
{
	struct objectProperty op;
	op.obj = obj;
	op.prop = prop;
	return mpt_process_vararg(format, va, processObjectArgs, &op);
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
		return obj->_vptr->setProperty(obj, prop, 0);
	}
	va_start(va, fmt);
	ret = mpt_object_vset(obj, prop, fmt, va);
	va_end(va);
	
	return ret;
}
