/*!
 * execute solver step
 */

#include "event.h"
#include "message.h"
#include "config.h"
#include "meta.h"

#include "client.h"

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
	if (!cl) {
		MPT_ABORT("missing client descriptor");
	}
	do {
		const char *arg = 0;
		if ((ret = it->_vptr->get(it, 's', &arg)) < 0) {
			return MPT_ERROR(BadValue);
		}
		if (!ret) {
			break;
		}
		if (!arg || !*arg) {
			continue;
		}
		mpt_path_set(&p, arg, -1);
		if (cl->_vptr->remove(cl, &p) < 0) {
			if (!log) {
				continue;
			}
			mpt_log(log, __func__, MPT_LOG(Info), "%s: %s",
			        MPT_tr("no config element"), arg);
		} else {
			if (!log) {
				continue;
			}
			mpt_log(log, __func__, MPT_LOG(Debug2), "%s: %s",
			        MPT_tr("removed config element"), arg);
		}
	} while ((ret = it->_vptr->advance(it)) > 0);
	
	return ret;
}
