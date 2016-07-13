/*!
 * solver client data operations
 */

#include "output.h"
#include "meta.h"

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
	MPT_INTERFACE(metatype) *m;
	MPT_INTERFACE(output) *out;
	
	if (!val) {
		out = pr->output;
		if (!out || pr->logger || !out->_vptr->obj.addref((void *) out)) {
			return 0;
		}
		if ((pr->logger = mpt_output_logger(out))) {
			return 1;
		}
		out->_vptr->obj.ref.unref((void *) out);
		return MPT_ERROR(BadOperation);
	}
	if (!val->fmt) {
		if ((m = pr->_mt)) {
			return m->_vptr->assign(m, val);
		}
		return 0;
	}
	if (val->fmt[0] == MPT_ENUM(TypeMeta)) {
		MPT_INTERFACE(metatype) *m, *o;
		m = 0;
		if (val->ptr && (m = *((void **) val->ptr))) {
			if (!(m = m->_vptr->clone(m))) {
				return MPT_ERROR(BadOperation);
			}
		}
		if ((o = pr->_mt)) {
			o->_vptr->ref.unref((void *) o);
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
		if (!pr->logger && out->_vptr->obj.addref((void *) out)) {
			if (!(pr->logger = mpt_output_logger(out))) {
				out->_vptr->obj.ref.unref((void *) out);
			}
		}
	}
	if (pr->output) {
		pr->output->_vptr->obj.ref.unref((void *) pr->output);
	}
	pr->output = out;
	
	return 1;
}
