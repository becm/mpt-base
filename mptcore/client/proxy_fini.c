/*!
 * solver client data operations
 */

#include <string.h>

#include "meta.h"
#include "output.h"

#include "client.h"

/*!
 * \ingroup mptClient
 * \brief clear proxy data
 * 
 * Clear resource bindings of proxy.
 * 
 * \param pr  proxy data pointer
 */
extern void mpt_proxy_fini(MPT_STRUCT(proxy) *pr)
{
	MPT_INTERFACE(metatype) *m;
	
	if ((m = pr->_ref)) {
		m->_vptr->ref.unref((void *) m);
		pr->_ref = 0;
	}
	pr->_hash = 0;
}
