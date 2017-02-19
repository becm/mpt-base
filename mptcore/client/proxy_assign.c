/*!
 * solver client data operations
 */

#include <string.h>

#include "object.h"
#include "meta.h"

#include "convert.h"

#include "client.h"

/*!
 * \ingroup mptClient
 * \brief set proxy data
 * 
 * Assign matching element in proxy data.
 * 
 * \param cd  data pointer
 * \param val source value
 * 
 * \return copy/reference result
 */
extern int mpt_proxy_assign(MPT_STRUCT(proxy) *pr, const char *name, MPT_INTERFACE(metatype) *src)
{
	MPT_INTERFACE(metatype) *m;
	MPT_INTERFACE(object) *o;
	int ret;
	
	if (!(m = pr->_ref)) {
		return MPT_ERROR(BadArgument);
	}
	if (!name) {
		MPT_STRUCT(value) val;
		const char *txt;
		size_t len;
		
		if (!src) {
			return m->_vptr->assign(m, 0);
		}
		if ((ret = src->_vptr->conv(m, 's', &txt)) > 0) {
			val.ptr = txt;
			len = txt ? strlen(txt) : 0;
		}
		else if ((ret = src->_vptr->conv(m, MPT_ENUM(TypeValue), &val)) < 0) {
			return ret;
		}
		else if (!val.fmt) {
			txt = val.ptr;
			len = txt ? strlen(val.ptr) : 0;
		}
		else if (!(txt = mpt_data_tostring(&val.ptr, *val.fmt, &len))) {
			return MPT_ERROR(BadType);
		}
		ret = m->_vptr->assign(m, &val);
		if (ret >= 0) {
			pr->_hash = len ? mpt_hash(txt, len) : 0;
		}
		return ret;
	}
	if ((ret = m->_vptr->conv(m, MPT_ENUM(TypeObject), &o)) <= 0) {
		return ret;
	}
	if (!o) {
		return 0;
	}
	return o->_vptr->setProperty(o, name, src);
}
