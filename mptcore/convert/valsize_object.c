/*!
 * \file
 * mpt type registry
 */

#include "convert.h"
#include "array.h"

#include <stdlib.h>

#include <sys/uio.h>

# define MPT_OBJECT_OFFSET (MPT_ENUM(_TypeDynamic) + MPT_ENUM(_TypeLimit))

static int object_count = 0;

/*!
 * \ingroup mptConvert
 * \brief register object
 * 
 * Register new object type to use with mpt_valsize()
 * and other internal type conversions.
 * 
 * \return type code of new object type
 */
extern int mpt_valtype_newobject()
{
	if (object_count >= MPT_ENUM(_TypeLimit)) {
		return MPT_ERROR(MissingBuffer);
	}
	return MPT_OBJECT_OFFSET + object_count++;
}
/*!
 * \ingroup mptConvert
 * \brief check object type
 * 
 * Test if type is object
 * 
 * \param type  typecode
 * 
 * \retval 0   builtin object type
 * \retval <0  no object type
 * \retval >0  user object type
 */
extern int mpt_valtype_isobject(int type)
{
	if (MPT_value_isObject(type)) {
		return 0;
	}
	if (type < MPT_OBJECT_OFFSET) {
		return MPT_ERROR(BadType);
	}
	type -= MPT_OBJECT_OFFSET;
	if (type < object_count) {
		return type + 1;
	}
	if (type <= MPT_ENUM(_TypeLimit)) {
		return MPT_ERROR(BadValue);
	}
	return MPT_ERROR(BadType);
}
