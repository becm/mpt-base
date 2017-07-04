/*!
 * MPT default client config operations
 */

#include "client.h"

__MPT_NAMESPACE_BEGIN

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
	return config::global()->assign(p, v);
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
	return config::global()->query(p);
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
	return config::global()->remove(p);
}

__MPT_NAMESPACE_END
