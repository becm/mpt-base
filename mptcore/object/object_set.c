/*!
 * set object data from arguments.
 */

#include <stdlib.h>

#ifdef MPT_NO_CONVERT
# include <string.h>
#endif

#include "meta.h"
#include "convert.h"

#include "object.h"

struct objectProperty
{
	MPT_INTERFACE(object) *obj;
	const char *prop;
};
struct metaScalar
{
	MPT_INTERFACE(metatype) _mt;
	MPT_STRUCT(scalar) val;
	char fmt[2];
};
/* reference interface */
static void unrefScalar(MPT_INTERFACE(reference) *ref)
{
	(void) ref;
	MPT_ABORT("unref scalar value interface");
}
static uintptr_t addrefScalar(MPT_INTERFACE(reference) *ref)
{
	(void) ref;
	MPT_ABORT("ref scalar value interface");
}
/* metatype interface */
static int convScalar(const MPT_INTERFACE(metatype) *mt, int type, void *ptr)
{
	const struct metaScalar *s = (void *) mt;
	const void *val = &s->val.val;
	if (!type) {
		if (ptr) *((const char **) ptr) = s->fmt;
		return MPT_ENUM(TypeMeta);
	}
#ifdef MPT_NO_CONVERT
	if (type == s->val.type) {
		if (ptr && s->val.len) memcpy(ptr, val, s->val.len);
		return MPT_ENUM(TypeMeta);
	}
	return MPT_ERROR(BadType);
#else
	return mpt_data_convert(&val, s->val.type, ptr, type);
#endif
}
static MPT_INTERFACE(metatype) *cloneScalar(const MPT_INTERFACE(metatype) *mt)
{
	const struct metaScalar *s = (void *) mt;
	struct metaScalar *c;
	if (!(c = malloc(sizeof(*c)))) {
		return 0;
	}
	*c = *s;
	return &c->_mt;
}
/* set object property from iterator */
static int processObjectArgs(void *ptr, MPT_INTERFACE(iterator) *it)
{
	const struct objectProperty *op = ptr;
	return mpt_object_set_iterator(op->obj, op->prop, it);
}
/* set object property to arguments */
static int processObjectFormat(MPT_INTERFACE(object) *obj, const char *prop, const char *fmt, va_list va)
{
	struct objectProperty op;
	int ret;
	if (!(ret = fmt[1])) {
		static const MPT_INTERFACE_VPTR(metatype) metaValCtl = {
		    { unrefScalar, addrefScalar },
		    convScalar,
		    cloneScalar
		};
		struct metaScalar s;
		va_list cpy;
		va_copy(cpy, va);
		ret = mpt_scalar_argv(&s.val, *fmt, cpy);
		va_end(cpy);
		if (ret < 0) {
			return ret;
		}
		s._mt._vptr = &metaValCtl;
		if ((ret = obj->_vptr->set_property(obj, prop, &s._mt)) >= 0
		    || ret != MPT_ERROR(BadType)) {
			return ret;
		}
	}
	op.obj = obj;
	op.prop = prop;
	return mpt_process_vararg(fmt, va, processObjectArgs, &op);
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
		return obj->_vptr->set_property(obj, prop, mpt_metatype_default());
	}
	return processObjectFormat(obj, prop, fmt, va);
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
		return obj->_vptr->set_property(obj, prop, mpt_metatype_default());
	}
	va_start(va, fmt);
	ret = processObjectFormat(obj, prop, fmt, va);
	va_end(va);
	
	return ret;
}
