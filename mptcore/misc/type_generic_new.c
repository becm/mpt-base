/*
 * MPT core library
 *   register extended generic type
 */

#include "core.h"

/*!
 * \ingroup mptCore
 * \brief new generic type
 * 
 * Create new type code in dynamic extended range.
 * To register explicit metatype, interface or
 * dynamic base type use supplied special functions.
 * 
 * \return new type code
 */
extern int mpt_type_generic_new(void)
{
	static int id = MPT_ENUM(_TypeGenericBase);
	if (id >= MPT_ENUM(_TypeGenericMax)) {
		return MPT_ERROR(BadValue);
	}
	return id++;
}
