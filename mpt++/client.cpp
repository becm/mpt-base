/*!
 * MPT default client config operations
 */

#include <stdlib.h>

#include "config.h"

#include "client.h"

__MPT_NAMESPACE_BEGIN

static metatype *cfg = 0;

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
    config *ptr = 0;
    if (cfg->conv(config::Type, &ptr) < 0) {
        return 0;
    }
    return ptr;
}
/*!
 * \ingroup mptClient
 * \brief convert from client
 * 
 * Get interfaces and data from client
 * 
 * \param type  target type code
 * \param ptr   conversion target address
 * 
 * \return conversion result
 */
int client::conv(int type, void *ptr) const
{
	int me = mpt_client_typeid();
	if (me < 0) {
	    me = Type;
	}
	if (!type) {
	    static const char fmt[] = { metatype::Type, config::Type, 0 };
	    if (ptr) *static_cast<const char **>(ptr) = fmt;
	    return me;
	}
	if (type == config::Type) {
	    if (ptr) *static_cast<class config **>(ptr) = clientConfig();
	    return me;
	}
	if (type == me) {
	    if (ptr) *static_cast<const client **>(ptr) = this;
	    return config::Type;
	}
	if (type == Type) {
	    if (ptr) *static_cast<const metatype **>(ptr) = this;
	    return config::Type;
	}
	return BadType;
}

__MPT_NAMESPACE_END
