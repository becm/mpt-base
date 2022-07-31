/*!
 * read basic and specific configuration from files.
 */

#include <string.h>

#include "meta.h"
#include "object.h"
#include "output.h"
#include "types.h"
#include "convert.h"

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
		const MPT_STRUCT(value) *val = args->_vptr->value(args);
		const void *ptr;
		const char *end;
		
		if (!val || !(ptr = val->ptr)) {
			if (info) {
				mpt_log(info, __func__, MPT_LOG(Error), "%s",
				        MPT_tr("unable to set default config"));
			}
			continue;
		}
		if (MPT_type_isConvertable(val->type)) {
			MPT_INTERFACE(convertable) *conv;
			
			if (!(conv = *((void * const *) ptr))) {
				if (info) {
					mpt_log(info, __func__, MPT_LOG(Error), "%s",
					        MPT_tr("missing convertable pointer"));
				}
				continue;
			}
			/* get assign target */
			else if ((res = conv->_vptr->convert(conv, MPT_ENUM(TypeProperty), &pr)) >= 0 && pr.val.type) {
				p.assign = 0;
				mpt_path_set(&p, pr.name, -1);
			}
			else if ((res = conv->_vptr->convert(conv, 's', &pr.name)) < 0) {
				if (info) {
					mpt_log(info, __func__, MPT_LOG(Error), "%s",
					        MPT_tr("invalid assinment"));
				}
				continue;
			}
			else if (!pr.name) {
				if (info) {
					mpt_log(info, __func__, MPT_LOG(Error), "%s",
					        MPT_tr("invalid property name"));
				}
				continue;
			}
		}
		else if (!(pr.name = mpt_data_tostring(&ptr, val->type, 0))) {
			if (info) {
				mpt_log(info, __func__, MPT_LOG(Warning), "%s: %d (%d)",
				        MPT_tr("bad string value"), ++count, val->type);
			}
			continue;
		}
		/* no property type assigned */
		if (!pr.val.type) {
			if (!(end = strchr(pr.name, '='))) {
				++err;
				if (info) {
					mpt_log(info, __func__, MPT_LOG(Warning), "%s: %s",
					        MPT_tr("missing path separator"), pr.name);
				}
				continue;
			}
			p.assign = '=';
			mpt_path_set(&p, pr.name, end - pr.name);
			MPT_property_set_string(&pr, end + 1);
		}
		/* no top level assign */
		if (!pr.name) {
			++err;
			if (info) {
				mpt_log(info, __func__, MPT_LOG(Warning), "%s: %d",
				        MPT_tr("missing config path"), ++count);
			}
			return count ? count : MPT_ERROR(BadType);
		}
		/* assign config */
		if (cfg->_vptr->assign(cfg, &p, &pr.val) < 0) {
			++err;
			if (info) {
				mpt_log(info, __func__, MPT_LOG(Error), "%s (%d): %d",
				        MPT_tr("config assign error"), val->type, ++count);
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

