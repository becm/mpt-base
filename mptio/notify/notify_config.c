/*!
 * register input with type system.
 */

#include <poll.h>

#include "config.h"
#include "output.h"

#include "notify.h"

/*!
 * \ingroup mptNotify
 * \brief apply connection settings
 * 
 * Add input settings from config to notifier.
 * 
 * \return added input count
 */
extern int mpt_notify_config(MPT_STRUCT(notify) *no, const MPT_INTERFACE(config) *cfg)
{
	static const char con[] = "mpt.connect";
	static const char bind[] = "mpt.listen";
	const MPT_INTERFACE(metatype) *mt;
	MPT_INTERFACE(input) *in;
	const char *ctl;
	int off, ret, ncon;
	
	off = cfg ? 4 : 0;
	
	ncon = 0;
	if ((mt = mpt_config_get(cfg, con + off, '.', 0))) {
		in = 0;
		ctl = 0;
		if (mt->_vptr->conv(mt, mpt_input_typeid(), &in) >= 0) {
			if (!in || !in->_vptr->meta.instance.addref((void *) in)) {
				mpt_log(0, __func__, MPT_LOG(Error), "%s",
				        "no valid connection reference");
				return in ? MPT_ERROR(BadOperation) : MPT_ERROR(BadValue);
			}
			else if ((ret = mpt_notify_add(no, POLLIN, in)) < 0) {
				return ret;
			} else {
				++ncon;
			}
		}
		else if (mt->_vptr->conv(mt, 's', &ctl) < 0) {
			return MPT_ERROR(BadType);
		}
		else if (ctl) {
			if ((ret = mpt_notify_connect(no, ctl)) < 0) {
				mpt_log(0, __func__, MPT_LOG(Error), "%s: %s",
				        "failed to connect to controller", ctl);
				return ret;
			} else {
				++ncon;
			}
		}
	}
	if ((mt = mpt_config_get(cfg, bind + off, '.', 0))) {
		in = 0;
		ctl = 0;
		if (mt->_vptr->conv(mt, mpt_input_typeid(), &in) >= 0) {
			if (!in || !in->_vptr->meta.instance.addref((void *) in)) {
				mpt_log(0, __func__, MPT_LOG(Error), "%s",
				        "no valid input reference");
				ret = in ? MPT_ERROR(BadOperation) : MPT_ERROR(BadValue);
			}
			else {
				ret = mpt_notify_add(no, POLLIN, in);
			}
		}
		else if (mt->_vptr->conv(mt, 's', &ctl) < 0) {
			ret = MPT_ERROR(BadType);
		}
		else if (!ctl) {
			return ncon;
		}
		else if ((ret = mpt_notify_bind(no, ctl, 2)) < 0) {
			mpt_log(0, __func__, MPT_LOG(Error), "%s: %s",
			        "failed to create controller", ctl);
		}
		if (!ncon && ret < 0) {
			ncon = ret;
		}
	}
	return ncon;
}
