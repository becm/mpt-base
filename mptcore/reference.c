/*!
 * operations on reference structure.
 */

#include "core.h"

/*!
 * \ingroup mptCore
 * \brief raise reference count
 * 
 * Check and increase reference registrations
 * 
 * \param ref  reference information
 */
extern uintptr_t mpt_reference_raise(MPT_STRUCT(reference) *ref)
{
	if (!ref->_val) return 0;
	if (++ref->_val) return ref->_val;
	--ref->_val;
	return 0;
}

/*!
 * \ingroup mptCore
 * \brief lower reference count
 * 
 * Check and decrease reference registrations
 * 
 * \param ref  reference information
 */
extern uintptr_t mpt_reference_lower(MPT_STRUCT(reference) *ref)
{
	if (!ref->_val) return 1;
	return --ref->_val;
}
