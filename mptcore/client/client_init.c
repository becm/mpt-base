/*!
 * initialize and terminate client instance.
 */

#include <stdlib.h>
#include <string.h>

#include "node.h"

#include "client.h"

/*!
 * \ingroup mptClient
 * \brief client data termination
 * 
 * Clear client data allocations and bindings.
 * 
 * \param solv  client descriptor
 */
extern void mpt_client_fini(MPT_INTERFACE(client) *solv)
{
	MPT_INTERFACE(metatype) *mt;
	
	if (solv->conf) {
		if (solv->conf->parent) {
			mpt_node_clear(solv->conf);
		} else {
			mpt_node_destroy(solv->conf);
		}
		solv->conf = 0;
	}
	if (!(mt = (void *) solv->out)) {
		return;
	}
	mt->_vptr->unref(mt);
	solv->out = 0;
}

/*!
 * \ingroup mptClient
 * \brief client data initialization
 * 
 * Set initial values for client descriptor data
 * 
 * \param solv  client descriptor
 */
extern void mpt_client_init(MPT_INTERFACE(client) *solv)
{
	solv->out = 0;
	solv->conf = 0;
}
