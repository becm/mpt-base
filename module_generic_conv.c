/*!
 * MPT module helper function
 *   convert from generic "metatype with object" interface
 */

#include "module.h"

/*!
 * \ingroup mptModule
 * \brief set module value
 * 
 * Convert to metatype or object interface
 * from module header
 * 
 * \param gen  module header
 * \param type target type
 * \param ptr  target address
 * 
 * \return conversion result
 */
extern int mpt_module_generic_conv(const MPT_STRUCT(module_generic) *gen, int type, void *ptr)
{
	if (!type) {
		static const char fmt[] = { MPT_ENUM(TypeObject), 0 };
		if (ptr) *((const char **) ptr) = fmt;
		return MPT_ENUM(TypeMeta);
	}
	if (type == MPT_ENUM(TypeObject)) {
		if (ptr) *((const void **) ptr) = &gen->_obj;
		return MPT_ENUM(TypeMeta);
	}
	if (type == MPT_ENUM(TypeMeta)) {
		if (ptr) *((const void **) ptr) = &gen->_mt;
		return MPT_ENUM(TypeObject);
	}
	return MPT_ERROR(BadType);
}
