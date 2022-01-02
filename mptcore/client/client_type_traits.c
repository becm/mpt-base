/*!
 * client metatype registration.
 */

#include "client.h"

#include "types.h"

/*!
 * \ingroup mptClient
 * \brief client properties
 * 
 * Get or register client metatype.
 * 
 * \return id for client metatype
 */
extern const MPT_STRUCT(named_traits) *mpt_client_type_traits(void)
{
	static const MPT_STRUCT(named_traits) *traits = 0;
	if (!traits) {
		traits = mpt_type_metatype_add("mpt.client");
	}
	return traits;
}
