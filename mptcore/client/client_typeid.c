/*!
 * client metatype registration.
 */

#include "client.h"

/*!
 * \ingroup mptClient
 * \brief client type
 * 
 * Get or register client metatype.
 * 
 * \return id for client metatype
 */
extern int mpt_client_typeid(void)
{
	static int id = 0;
	if (!id) {
		id = mpt_type_meta_new("client");
	}
	return id;
}
