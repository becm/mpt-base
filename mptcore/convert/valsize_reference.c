/*!
 * \file
 * mpt type registry
 */

#include "convert.h"
#include "array.h"

#include <stdlib.h>

#include <sys/uio.h>

static int ref_count = 0;

/*!
 * \ingroup mptConvert
 * \brief register reference
 * 
 * Register new reference type to use with mpt_valsize()
 * and other internal type conversions.
 * 
 * \return type code of new reference type
 */
extern int mpt_valtype_newref()
{
	if (ref_count >= MPT_ENUM(_TypeLimit)) {
		return MPT_ERROR(MissingBuffer);
	}
	return MPT_ENUM(_TypeDynamic) + ref_count++;
}
/*!
 * \ingroup mptConvert
 * \brief check reference type
 * 
 * Test if type is reference
 * 
 * \param type  typecode
 * 
 * \retval 0   builtin reference type
 * \retval <0  no reference type
 * \retval >0  user reference type
 * \retval >_TypeLimit  user object type
 */
extern int mpt_valtype_isref(int type)
{
	int obj;
	if (MPT_value_isUnrefable(type)) {
		return 0;
	}
	if (type < MPT_ENUM(_TypeDynamic)) {
		return MPT_ERROR(BadType);
	}
	if ((obj = mpt_valtype_isobject(type)) > 0) {
		return MPT_ENUM(_TypeLimit) + obj;
	}
	type -= MPT_ENUM(_TypeDynamic);
	if (type < ref_count) {
		return type + 1;
	}
	if (type <= MPT_ENUM(_TypeLimit)) {
		return MPT_ERROR(BadValue);
	}
	return MPT_ERROR(BadType);
}
