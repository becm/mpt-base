/*!
 * MPT default client config operations
 */

#include <stdlib.h>

#include "client.h"

__MPT_NAMESPACE_BEGIN

static config *cfg = 0;

static void unrefConfig()
{
	if (cfg) {
		cfg->unref();
		cfg = 0;
	}
}
static config *clientConfig()
{
	static const char dest[] = "mpt.client\0";
	static path p;
	
	p.set(dest);
	if (!cfg) {
		atexit(unrefConfig);
		cfg = config::global(&p);
	}
	return cfg;
}
/*!
 * \ingroup mptClient
 * \brief set client config
 * 
 * Assign global config element
 * 
 * \param p  element location
 * \param v  new element value
 * 
 * \return assignment result
 */
int client::assign(const path *p, const value *v)
{
	config *cfg = clientConfig();
	return cfg ? cfg->assign(p, v) : BadArgument;
}
/*!
 * \ingroup mptClient
 * \brief get client config
 * 
 * Get global config element
 * 
 * \param p  element location
 * 
 * \return metatype at location
 */
const metatype *client::query(const path *p) const
{
	config *cfg = clientConfig();
	return cfg ? cfg->query(p) : 0;
}
/*!
 * \ingroup mptClient
 * \brief clear client config
 * 
 * Remove global config element
 * 
 * \param p  element location
 * 
 * \retval <0  on error
 * \retval >=0 on success
 */
int client::remove(const path *p)
{
	config *cfg = clientConfig();
	return cfg ? cfg->remove(p) : 0;
}

__MPT_NAMESPACE_END
