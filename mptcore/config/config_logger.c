
#include "meta.h"

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
	const MPT_INTERFACE(metatype) *mt;
	MPT_INTERFACE(logger) *info = 0;
	
	if (!cfg) {
		/* search global logger */
		if ((mt = mpt_config_get(0, "mpt.logger", '.', 0))
		    && mt->_vptr->conv(mt, MPT_type_pointer(MPT_ENUM(TypeLogger)), &info) >= 0
		    && info) {
			return info;
		}
		/* fallback to compatible 'output' */
		if ((mt = mpt_config_get(0, "mpt.output", '.', 0))) {
			mt->_vptr->conv(mt, MPT_type_pointer(MPT_ENUM(TypeLogger)), &info);
		}
		return info;
	}
	/* try direct interface */
	if ((mt = cfg->_vptr->query(cfg, 0))
	    && mt->_vptr->conv(mt, MPT_type_pointer(MPT_ENUM(TypeLogger)), &info) >= 0
	    && info) {
		return info;
	}
	/* search local logger */
	if ((mt = mpt_config_get(cfg, "logger", 0, 0))
	    && mt->_vptr->conv(mt, MPT_type_pointer(MPT_ENUM(TypeLogger)), &info) >= 0
	    && info) {
		return info;
	}
	/* fallback to compatible 'output' */
	if ((mt = mpt_config_get(cfg, "output", 0, 0))) {
		mt->_vptr->conv(mt, MPT_type_pointer(MPT_ENUM(TypeLogger)), &info);
	}
	return info;
}
