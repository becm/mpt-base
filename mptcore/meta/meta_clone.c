
#include <string.h>
#include <sys/uio.h>

#include "core.h"

int propertyLength(MPT_STRUCT(property) *pr, struct iovec *vec, int dlen)
{
	static const char fmt[2] = { MPT_ENUM(TypeVector) };
	
	if (pr->fmt) switch (*pr->fmt) {
	  case 0:
		vec->iov_base = (void *) pr->data;
		vec->iov_len  = dlen;
		pr->fmt  = fmt;
		pr->data = vec;
	  case (char) MPT_ENUM(TypeVector):
		return 1;
	  case 's':
	  case 'k':
		pr->fmt  = 0;
		pr->data = *(void **) pr->data;
		break;
	  default:
		return -1;
	}
	vec->iov_len = (vec->iov_base = (void*) pr->data) ? strlen(pr->data) : 0;
	return 0;
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
	switch (dlen) {
	/* immediate types */
	  case 's': dlen = strlen(pr.data) + 1; break;
	  case MPT_ENUM(TypeVector): dlen = ((struct iovec *) (pr.data))->iov_len; break;
	/* length from description */
	  default:
		if ((dlen = propertyLength(&pr, &vec, dlen)) < 0) {
			return 0;
		} else {
			dlen = dlen ? vec.iov_len : vec.iov_len+1;
		}
	}
	if (!(meta = mpt_meta_new(dlen))) {
		return 0;
	}
	pr.name = "";
	pr.desc = 0;
	if (!dlen || (dlen = mpt_meta_pset(meta, &pr, 0)) >= 0)
		return meta;
	
	meta->_vptr->unref(meta);
	
	return 0;
}
