
#include <string.h>
#include <sys/uio.h>

#include "convert.h"

#include "core.h"

int valueLength(MPT_STRUCT(value) *val, struct iovec *vec, int dlen)
{
	static const char fmt[2] = { (char) MPT_ENUM(TypeVector) };
	
	if (!val->fmt) {
		return val->ptr ? strlen(val->ptr) + 1 : 0;
	}
	if (*val->fmt & MPT_ENUM(TypeVector)) {
		vec = (void *) val->ptr;
		return vec ? vec->iov_len : 0;
	}
	switch (*val->fmt) {
	  case 0:
		vec->iov_base = (void *) val->ptr;
		vec->iov_len  = dlen;
		val->fmt = fmt;
		val->ptr = vec;
		return dlen;
	  case 's':
	  case 'k':
		val->fmt = 0;
		val->ptr = *(void **) val->ptr;
		return val->ptr ? strlen(val->ptr) + 1 : 0;
	  default:
		return mpt_valsize(*val->fmt);
	}
}

/*!
 * \ingroup mptMeta
 * \brief clone metatype
 * 
 * create generic type with same identifier and data
 * 
 * \param meta source metatype
 * 
 * \return created metatype
 */

extern MPT_INTERFACE(metatype) *mpt_meta_clone(MPT_INTERFACE(metatype) *meta)
{
	struct iovec vec;
	MPT_STRUCT(property) pr;
	int dlen;
	
	/* get metatype data */
	pr.name = "";
	pr.desc = 0;
	if ((dlen = meta->_vptr->property(meta, &pr, 0)) < 0) {
		return 0;
	}
	if ((dlen = valueLength(&pr.val, &vec, dlen)) < 0) {
		return 0;
	}
	if (!(meta = mpt_meta_new(dlen))) {
		return 0;
	}
	pr.name = "";
	pr.desc = 0;
	if (!dlen || (dlen = mpt_meta_pset(meta, &pr, 0)) >= 0) {
		return meta;
	}
	meta->_vptr->unref(meta);
	
	return 0;
}
