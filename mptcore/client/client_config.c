/*!
 * config assignment from arguments
 */

#include "output.h"
#include "types.h"
#include "convert.h"

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
extern int mpt_client_config(MPT_INTERFACE(config) *cfg, MPT_INTERFACE(logger) *info)
{
	MPT_INTERFACE(iterator) *it;
	const MPT_STRUCT(value) *val;
	const void *ptr;
	const char *str;
	int ret;
	
	it = 0;
	if ((ret = mpt_config_get(0, "mpt.args", MPT_ENUM(TypeIteratorPtr), &it)) <= 0
	    || !it) {
		return 0;
	}
	/* set config file */
	if (!(val = it->_vptr->value(it))) {
		mpt_log(info, __func__, MPT_LOG(Info), "%s",
		        MPT_tr("no client config filename"));
		return 0;
	}
	ptr = val->_addr;
	if (!(str = mpt_data_tostring(&ptr, val->_type, 0))) {
		mpt_log(info, __func__, MPT_LOG(Error), "%s",
		        MPT_tr("bad client config filename"));
		return ret;
	}
	if ((ret = mpt_config_set(cfg, 0, str, 0, 0)) < 0) {
		mpt_log(info, __func__, MPT_LOG(Error), "%s: %s",
		        MPT_tr("failed to set client config"), str);
		return ret;
	}
	if (!info) {
		info = mpt_log_default();
	}
	/* apply config settings */
	if ((ret = it->_vptr->advance(it)) > 0
	    && (ret = mpt_config_args(cfg, it, info))) {
		if (ret >= 0) {
			++ret;
		}
	}
	return ret;
}
