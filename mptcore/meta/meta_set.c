/*!
 * set MPT node metatype data.
 */

#include "object.h"
#include "config.h"

#include "meta.h"

/*!
 * \ingroup mptMeta
 * \brief set metatype data
 * 
 * Assign new value to compatible metatype
 * or create new element.
 * 
 * \param mptr  metatype pointer reference
 * \param val   value to assign
 * 
 * \return type of created meta element
 */
extern int mpt_meta_set(MPT_INTERFACE(metatype) **mptr, const MPT_STRUCT(value) *val)
{
	
	MPT_INTERFACE(metatype) *old, *mt;
	int ret = 0;
	
	if ((old = *mptr)) {
		/* try assign existing data */
		MPT_INTERFACE(object) *obj;
		MPT_INTERFACE(config) *cfg;
		
		if ((ret = old->_vptr->conv(old, 0, 0)) < 0) {
			ret = 0;
		}
		obj = 0;
		if ((old->_vptr->conv(old, MPT_ENUM(TypeObject), &obj)) >= 0
	            && obj) {
			int err;
			if (val) {
				MPT_STRUCT(value) tmp = *val;
				if ((err = mpt_object_set_value(obj, 0, &tmp)) < 0) {
					return err;
				}
				return ret;
			}
			if ((err = obj->_vptr->set_property(obj, 0, 0)) >= 0) {
				return ret;
			}
		}
		cfg = 0;
		if ((old->_vptr->conv(old, MPT_ENUM(TypeConfig), &cfg)) >= 0
		    && cfg
		    && (ret = cfg->_vptr->assign(cfg, 0, val)) >= 0) {
			return ret;
		}
	}
	/* default config data */
	if (!val) {
		/* try to reset existing iterator */
		MPT_INTERFACE(iterator) *it = 0;
		if (old
		    && old->_vptr->conv(old, MPT_ENUM(TypeIterator), &it) >= 0
		    && it
		    && it->_vptr->reset(it) >= 0) {
			return ret;
		}
		mt = mpt_metatype_default();
		ret = 0;
	}
	/* create new metatype for data */
	else if (!(mt = mpt_meta_new(*val))) {
		return MPT_ERROR(BadOperation);
	}
	else if ((ret = mt->_vptr->conv(mt, 0, 0)) < 0) {
		ret = 0;
	}
	if (old) {
		old->_vptr->ref.unref((void *) old);
	}
	*mptr = mt;
	return ret;
}

