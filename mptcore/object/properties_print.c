/*!
 * loop trough object/metatype properties
 */

#include <string.h>
#include <sys/uio.h>

#include "convert.h"

#include "object.h"

struct outData
{
	MPT_TYPE(property_handler) h;
	void *p;
};

static ssize_t saveString(void *ctx, const char *str, size_t len)
{
	struct iovec *vec = ctx;
	char *to;
	
	if (len >= vec->iov_len) {
		return MPT_ERROR(MissingData);
	}
	to = memcpy(vec->iov_base, str, len);
	
	vec->iov_base = to + len;
	vec->iov_len -= len;
	
	return len;
}

static int mprint(void *data, const MPT_STRUCT(property) *prop)
{
	char buf[1024];
	struct iovec vec;
	struct outData *par = data;
	MPT_STRUCT(property) pr = *prop;
	int len;
	
	*buf = 0;
	
	if (!pr.val.fmt) {
		if (!pr.val.ptr) { pr.val.ptr = buf; *buf = 0; }
		return par->h(par->p, &pr);
	}
	if (!pr.val.ptr) {
		return *pr.val.fmt ? -1 : 0;
	}
	vec.iov_base = buf;
	vec.iov_len  = sizeof(buf);
	if ((len = mpt_tostring(&pr.val, saveString, &vec))) {
		return par->h(par->p, prop);
	}
	buf[sizeof(buf) - vec.iov_len] = 0;
	pr.val.ptr = buf;
	pr.val.fmt = 0;
	
	return par->h(par->p, &pr);
}

/*!
 * \ingroup mptObject
 * \brief print properties
 * 
 * Process object properties matching traverse types.
 * 
 * \param get   property query operation
 * \param obj   property source
 * \param proc  process string/unprintable property
 * \param data  argument for property processing
 * \param match properties to process
 * 
 * \return index of requested property
 */
extern int mpt_properties_print(int (*get)(void *, MPT_STRUCT(property) *), void *obj, MPT_TYPE(property_handler) proc, void *data, int mask)
{
	struct outData par;
	
	par.h = proc;
	par.p = data;
	
	return mpt_properties_foreach(get, obj, mprint, &par, mask);
}
