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

struct object_property
{
	MPT_INTERFACE(object) *obj;
	const char *prop;
};
struct meta_scalar
{
	MPT_INTERFACE(metatype) _mt;
	MPT_STRUCT(scalar) val;
	char fmt[2];
};
/* convertable interface */
static int conv_scalar(MPT_INTERFACE(convertable) *conv, int type, void *ptr)
{
	const struct meta_scalar *s = (void *) conv;
	const void *val = &s->val.val;
	if (!type) {
		if (ptr) {
			*((const char **) ptr) = s->fmt;
			return 0;
		}
		return s->val.type;
	}
#ifdef MPT_NO_CONVERT
	if (type == s->val.type) {
		if (ptr && s->val.len) memcpy(ptr, val, s->val.len);
		return type;
	}
#else
	if (mpt_data_convert(&val, s->val.type, ptr, type) >= 0) {
		return s->val.type;
	}
#endif
	return MPT_ERROR(BadType);
}
/* metatype interface */
static void unref_scalar(MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	MPT_ABORT("unref scalar value interface");
}
static uintptr_t addref_scalar(MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	MPT_ABORT("ref scalar value interface");
}
static MPT_INTERFACE(metatype) *clone_scalar(const MPT_INTERFACE(metatype) *mt)
{
	const struct meta_scalar *s = (void *) mt;
	struct meta_scalar *c;
	if (!(c = malloc(sizeof(*c)))) {
		return 0;
	}
	*c = *s;
	return &c->_mt;
}
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
		static const MPT_INTERFACE_VPTR(metatype) metaValCtl = {
		    { conv_scalar },
		    unref_scalar,
		    addref_scalar,
		    clone_scalar
		};
		struct meta_scalar s;
		va_list cpy;
		va_copy(cpy, va);
		ret = mpt_scalar_argv(&s.val, *fmt, cpy);
		va_end(cpy);
		if (ret < 0) {
			return ret;
		}
		s._mt._vptr = &metaValCtl;
		if ((ret = obj->_vptr->set_property(obj, prop, (MPT_INTERFACE(convertable) *) &s._mt)) >= 0
		    || ret != MPT_ERROR(BadType)) {
			return ret;
		}
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
