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
	MPT_INTERFACE(output) *o;
	MPT_INTERFACE(logger) *l;
	
	if ((m = pr->_mt)) {
		m->_vptr->unref(m);
		pr->_mt = 0;
	}
	if ((o = pr->output)) {
		o->_vptr->obj.unref((void *) o);
		pr->output = 0;
	}
	if ((l = pr->logger)) {
		l->_vptr->unref(l);
		pr->logger = 0;
	}
	pr->hash = 0;
}