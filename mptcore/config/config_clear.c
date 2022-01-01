/*!
 * execute solver step
 */

#include "meta.h"
#include "types.h"
#include "convert.h"
#include "output.h"

#include "config.h"

/*!
 * \ingroup mptClient
 * \brief config clear
 * 
 * Remove elements of config.
 * 
 * \param cl  config descriptor
 * \param it  event data
 * 
 * \return hint to event controller (stop/continue/error)
 */
extern int mpt_config_clear(MPT_INTERFACE(config) *cl, MPT_INTERFACE(iterator) *it, MPT_INTERFACE(logger) *log)
{
	MPT_STRUCT(path) p = MPT_PATH_INIT;
	int ret;
	
	if (!it) {
		return cl->_vptr->remove(cl, 0);
	}
	do {
		const MPT_STRUCT(value) *val = it->_vptr->value(it);
		const void *ptr;
		const char *arg = 0;
		
		if (!val) {
			if (log) {
				mpt_log(log, __func__, MPT_LOG(Info), "%s: %s",
				        MPT_tr("no config element"), arg);
			}
			continue;
		}
		ptr = val->ptr;
		if (MPT_type_isConvertable(val->type)) {
			MPT_INTERFACE(convertable) *conv = *((void * const *) ptr);
			
			if (!conv || (ret = conv->_vptr->convert(conv, 's', &arg)) < 0) {
				if (log) {
					mpt_log(log, __func__, MPT_LOG(Info), "%s: %s",
					        MPT_tr("no config element"), arg);
				}
				continue;
			}
		}
		else if (!(arg = mpt_data_tostring(&ptr, val->type, 0))) {
			if (log) {
				mpt_log(log, __func__, MPT_LOG(Info), "%s: %s",
				        MPT_tr("no config element"), arg);
			}
			continue;
		}
		if (!arg || !*arg) {
			continue;
		}
		mpt_path_set(&p, arg, -1);
		if (cl->_vptr->remove(cl, &p) < 0) {
			if (log) {
				mpt_log(log, __func__, MPT_LOG(Info), "%s: %s",
				        MPT_tr("no config element"), arg);
			}
		} else {
			if (log) {
				mpt_log(log, __func__, MPT_LOG(Debug2), "%s: %s",
				        MPT_tr("removed config element"), arg);
			}
		}
	} while ((ret = it->_vptr->advance(it)) > 0);
	
	return ret;
}
