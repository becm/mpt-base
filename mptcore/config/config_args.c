/*!
 * read basic and specific configuration from files.
 */

#include <string.h>

#include "meta.h"

#include "config.h"

/*!
 * \ingroup mptClient
 * \brief set config elements
 * 
 * Assign config to properties or strings from iterator.
 * 
 * \param cfg   configuration target
 * \param args  elements to set
 * 
 * \return error or offset of first failed element
 */
extern int mpt_config_args(MPT_INTERFACE(config) *cfg, MPT_INTERFACE(iterator) *args, MPT_INTERFACE(logger) *info)
{
	MPT_STRUCT(path) p = MPT_PATH_INIT;
	
	int res, count, err;
	
	if (!args) {
		res = cfg->_vptr->assign(cfg, &p, 0);
		if (res < 0) {
			if (info) {
				mpt_log(info, __func__, MPT_LOG(Error), "%s",
				        MPT_tr("unable to set default config"));
			}
			return res;
		}
		return 0;
	}
	err = 0;
	count = 0;
	do {
		MPT_STRUCT(property) pr = MPT_PROPERTY_INIT;
		
		/* get assign target */
		if ((res = args->_vptr->get(args, MPT_ENUM(TypeProperty), &pr)) >= 0) {
			p.assign = 0;
			mpt_path_set(&p, pr.name, -1);
		}
		else if ((res = args->_vptr->get(args, 's', &pr.name)) >= 0) {
			const char *end;
			
			if (!(end = pr.name)) {
				++err;
				if (info) {
					mpt_log(info, __func__, MPT_LOG(Warning), "%s: %d",
					        MPT_tr("bad string value"), ++count);
					continue;
				}
				return count ? count : MPT_ERROR(BadValue);
			}
			if (!(end = strchr(end, '='))) {
				++err;
				if (info) {
					mpt_log(info, __func__, MPT_LOG(Warning), "%s: %s",
					        MPT_tr("missing path separator"), pr.name);
					continue;
				}
				return count ? count : MPT_ERROR(BadValue);
			}
			p.assign = '=';
			mpt_path_set(&p, pr.name, end - pr.name);
			pr.val.fmt = 0;
			pr.val.ptr = end + 1;
		}
		else {
			if (info) {
				int type = args->_vptr->get(args, 0, 0);
				mpt_log(info, __func__, MPT_LOG(Warning), "%s (%d): %d",
				        MPT_tr("bad source type"), type, ++count);
				continue;
			}
			return count ? count : MPT_ERROR(BadType);
		}
		/* no top level assign */
		if (!pr.name) {
			++err;
			if (info) {
				mpt_log(info, __func__, MPT_LOG(Warning), "%s: %d",
				        MPT_tr("missing config path"), ++count);
				continue;
			}
			return count ? count : MPT_ERROR(BadType);
		}
		/* assign config */
		if (cfg->_vptr->assign(cfg, &p, &pr.val) < 0) {
			++err;
			if (info) {
				int type = args->_vptr->get(args, 0, 0);
				mpt_log(info, __func__, MPT_LOG(Error), "%s (%d): %d",
				        MPT_tr("config assign error"), type, ++count);
				continue;
			}
			return count ? count : MPT_ERROR(BadValue);
		}
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Debug2), "%s: %s",
			        MPT_tr("element assiged"), pr.name);
			continue;
		}
		++count;
	} while ((res = args->_vptr->advance(args)) > 0);
	
	return res ? count : 0;
}

