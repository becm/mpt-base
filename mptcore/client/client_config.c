/*!
 * config assignment from arguments
 */

#include "meta.h"

#include "config.h"

/*!
 * \ingroup mptClient
 * \brief config from arguments
 * 
 * Set config arguments form saved unprocessed arguments
 * by e.g. `mpt_init()`.
 * 
 * \param cfg  client config
 * 
 * \return error or failed config assignment position
 */
extern int mpt_client_config(MPT_INTERFACE(config) *cfg)
{
	const MPT_INTERFACE(metatype) *mt;
	MPT_INTERFACE(iterator) *it;
	const char *val;
	int ret;
	
	it = 0;
	if (!(mt = mpt_config_get(0, "mpt.args", '.', 0))
	    || (ret = mt->_vptr->conv(mt, MPT_ENUM(TypeIterator), &it)) <= 0
	    || !it) {
		return 0;
	}
	/* set config file */
	if ((ret = it->_vptr->get(it, 's', &val)) <= 0) {
		return ret;
	}
	if ((ret = mpt_config_set(cfg, 0, val, 0, 0)) < 0) {
		return ret;
	}
	/* apply config settings */
	if ((ret = it->_vptr->advance(it)) > 0
	    && (ret = mpt_config_args(cfg, it))) {
		if (ret >= 0) {
			++ret;
		}
	}
	return ret;
}