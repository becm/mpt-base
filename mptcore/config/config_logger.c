/*!
 * 
 * MPT core library
 *   get suitable logger from configuration
 */

#include "types.h"

#include "config.h"

/*!
 * \ingroup mptConfig
 * \brief get log interface
 * 
 * Find logger element in configuration tree/list.
 * 
 * \param cfg  element path
 * 
 * \return config element if exists
 */
extern MPT_INTERFACE(logger) *mpt_config_logger(const MPT_INTERFACE(config) *cfg)
{
	MPT_INTERFACE(logger) *info = 0;
	
	if (!cfg) {
		/* search global logger */
		if ((mpt_config_get(0, "mpt.logger", MPT_ENUM(TypeLoggerPtr), &info)) >= 0
		    && info) {
			return info;
		}
		/* fallback to compatible 'output' */
		mpt_config_get(0, "mpt.output", MPT_ENUM(TypeLoggerPtr), &info);
		return info;
	}
	/* try direct interface */
	if ((mpt_config_getp(cfg, 0, MPT_ENUM(TypeLoggerPtr), &info)) >= 0
	    && info) {
		return info;
	}
	/* search local logger */
	if ((mpt_config_get(cfg, "logger", MPT_ENUM(TypeLoggerPtr), &info)) >= 0
	    && info) {
		return info;
	}
	/* fallback to compatible 'output' */
	mpt_config_get(cfg, "output", MPT_ENUM(TypeLoggerPtr), &info);
	return info;
}
