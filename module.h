/*!
 * general module helpers
 */

#include "mptcore/meta.h"
#include "mptcore/object.h"


MPT_STRUCT(module_value)
{
#ifdef __cplusplus
	inline module_value() : _it(0)
	{ }
protected:
#else
# define MPT_MODULE_VALUE_INIT { 0, MPT_VALUE_INIT }
#endif
	MPT_INTERFACE(iterator) *_it;
	MPT_STRUCT(value) _val;
};

#ifdef __cplusplus
class module_generic : public metatype, public object
{ }
#else
MPT_STRUCT(module_generic)
{
	MPT_INTERFACE(metatype) _mt;
	MPT_INTERFACE(object)   _obj;
};
#endif

__MPT_EXTDECL_BEGIN

/* module header type conversion */
extern int mpt_module_header_conv(const MPT_STRUCT(module_generic) *, int , void *);

/* get value and advance source */
extern int mpt_module_value_init(MPT_STRUCT(module_value) *, const MPT_INTERFACE(metatype) *);
extern int mpt_module_value_double(MPT_STRUCT(module_value) *, double *);
extern int mpt_module_value_uint(MPT_STRUCT(module_value) *, uint32_t *);
extern int mpt_module_value_int(MPT_STRUCT(module_value) *, int32_t *);
extern int mpt_module_value_key(MPT_STRUCT(module_value) *, const char **);

__MPT_EXTDECL_END
