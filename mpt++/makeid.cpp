/*
 * MPT C++ type identifier
 */

#include "core.h"

__MPT_NAMESPACE_BEGIN

/*!
 * \ingroup mptCore
 * \brief new generic type
 * 
 * Create new type code in dynamic extended range.
 * To register explicit metatype, interface or base type
 * use supplied special functions.
 * 
 * \return new type code
 */
extern int make_id()
{
	return mpt_type_generic_new();
}

__MPT_NAMESPACE_END
