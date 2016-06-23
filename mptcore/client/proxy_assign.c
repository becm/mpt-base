/*!
 * solver client data operations
 */

#include "message.h"

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
extern int mpt_proxy_assign(MPT_STRUCT(proxy) *pr, const MPT_STRUCT(value) *val)
{
	MPT_INTERFACE(output) *out;
	
	if (!val) {
		out = pr->out;
		if (!out || pr->log || !out->_vptr->obj.addref((void *) out)) {
			return 0;
		}
		if ((pr->log = mpt_output_logger(out))) {
			return 1;
		}
		out->_vptr->obj.unref((void *) out);
		return MPT_ERROR(BadOperation);
	}
	if (!val->fmt) {
		if ((out = pr->out)) {
			return mpt_object_pset((void *) out, 0, val, 0);
		}
		return 0;
	}
	if (val->fmt[0] == MPT_ENUM(TypeMeta)) {
		MPT_INTERFACE(metatype) *m = 0, *o;
		if (val->ptr && (m = *((void **) val->ptr))) {
			if (!(m = m->_vptr->clone(m))) {
				return MPT_ERROR(BadOperation);
			}
		}
		if ((o = pr->_mt)) {
			o->_vptr->unref(o);
		}
		pr->_mt = m;
		pr->hash = 0;
		
		return 1;
	}
	if (val->fmt[0] != MPT_ENUM(TypeOutput)) {
		return MPT_ERROR(BadType);
	}
	out = 0;
	if (val->ptr && (out = *((void **) val->ptr))) {
		if (!out->_vptr->obj.addref((void *) out)) {
			return MPT_ERROR(BadOperation);
		}
		if (!pr->log && out->_vptr->obj.addref((void *) out)) {
			if (!(pr->log = mpt_output_logger(out))) {
				out->_vptr->obj.unref((void *) out);
			}
		}
	}
	if (pr->out) {
		pr->out->_vptr->obj.unref((void *) pr->out);
	}
	pr->out = out;
	
	return 1;
}
