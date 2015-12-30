
#include <errno.h>
#include <string.h>
#include <sys/uio.h>

#include "array.h"
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
	MPT_STRUCT(value) val;
	size_t dlen;
	char valfmt[2];
	
	valfmt[1] = 0;
	
	/* normal string */
	if ((val.ptr = meta->_vptr->typecast(meta, *valfmt = 's'))) {
		val.fmt = 0;
		dlen = strlen(val.ptr);
	}
	/* character array */
	else if ((val.ptr = meta->_vptr->typecast(meta, *valfmt = 'C'))) {
		const MPT_STRUCT(array) *a = val.ptr;
		val.fmt = valfmt;
		dlen = a->_buf ? a->_buf->used : 0;
	}
	/* character vector */
	else if ((val.ptr = meta->_vptr->typecast(meta, *valfmt = (char) ('c' | MPT_ENUM(TypeVector))))) {
		val.fmt = valfmt;
		dlen = ((const struct iovec *) val.ptr)->iov_len;
	}
	/* invalid data type */
	else {
		errno = EINVAL;
		return 0;
	}
	/* new metatype of required length */
	if (!(meta = mpt_meta_new(dlen))) {
		return 0;
	}
	val.fmt = valfmt;
	
	/* set text data */
	if (meta->_vptr->assign(meta, &val) < 0) {
		meta->_vptr->unref(meta);
		return 0;
	}
	return meta;
}
