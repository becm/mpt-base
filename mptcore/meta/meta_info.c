/*!
 * log metatype info.
 */

#include "object.h"

#include "meta.h"

/*!
 * \ingroup mptMeta
 * \brief metatype info
 * 
 * Output metatype information.
 * 
 * \param mt      metatype instance
 * \param src     source for metatype log info
 * \param mode    log flags
 * \param action  action taken on metatype
 * \param info    logger interface
 */
void mpt_meta_info(const MPT_INTERFACE(metatype) *mt, const char *src, int mode, const char *action, MPT_INTERFACE(logger) *info)
{
	const char *type, *desc;
	int code;
	
	/* default value setup */
	if (!src) {
		src = __func__;
	}
	if (mode < 0) {
		mode = MPT_LOG(Info);
	}
	
	code = mt->_vptr->conv(mt, 0, 0);
	
	/* interface instance */
	if ((desc = mpt_interface_typename(code))) {
		MPT_INTERFACE(object) *obj = 0;
		
		if (!*desc) {
			desc = 0;
		}
		/* object specific name */
		if (mt->_vptr->conv(mt, MPT_ENUM(TypeObject), &obj) >= 0
		    && obj
		    && (type = mpt_object_typename(obj))) {
			desc = type;
			type = MPT_tr("object");
		} else {
			type = MPT_tr("interface");
		}
	}
	/* generic instance */
	else {
		type = MPT_tr("metatype");
		
		/* output typecode */
		if (!(desc = mpt_meta_typename(code)) || !*desc) {
			if (action) {
				mpt_log(info, src, mode, "%s: %s: %d",
				        action, type, code);
			} else {
				mpt_log(info, src, mode, "%s: %d",
				        type, code);
			}
			return;
		}
	}
	if (action) {
		if (desc) {
			mpt_log(info, src, mode, "%s: %s: %s",
			        action, type, desc);
		} else {
			mpt_log(info, src, mode, "%s: %s",
			        action, type);
		}
		return;
	}
	if (desc) {
		mpt_log(info, src, mode, "%s: %s",
		        type, desc);
	} else {
		mpt_log(info, src, mode, "%s",
		        type);
	}
}
