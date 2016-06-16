/*!
 * initialize solver from shared library.
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "client.h"

/*!
 * \ingroup mptLoader
 * \brief proxy type pointer
 * 
 * Get 
 * 
 * \param pr  proxy metatype information
 * \param t   target cast type
 * 
 * \return metatype proxy pointer
 */
extern void *mpt_proxy_cast(const MPT_STRUCT(proxy) *pr, int t)
{
	MPT_INTERFACE(metatype) *m = 0;
	size_t i = 0;
	int c;
	
	while ((c = pr->_types[i++]) && i < sizeof(pr->_types)) {
		if (c == t) {
			return pr->_ref;
		}
		if (c == MPT_ENUM(TypeMeta)) {
			m = pr->_ref;
		}
	}
	if (!m) {
		return m;
	}
	return m->_vptr->conv(m, t, &m) < 0 ? 0 : m;
}