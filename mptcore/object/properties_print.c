/*!
 * loop trough object/metatype properties
 */

#include <string.h>
#include <sys/uio.h>

#include "convert.h"
#include "types.h"

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
	const void *ptr;
	const struct outData *par = data;
	const char *text;
	size_t avail = 0;
	
	if (prop 
	 && prop->val.type
	 && (ptr = prop->val.ptr)
	 && !(text = mpt_data_tostring(&ptr, prop->val.type, &avail))) {
		char buf[1024];
		struct iovec vec;
		int len;
		vec.iov_base = buf;
		vec.iov_len  = sizeof(buf);
		*buf = 0;
		
		if ((len = mpt_tostring(&prop->val, saveString, &vec)) >= 0) {
			MPT_STRUCT(property) pr = MPT_PROPERTY_INIT;
			pr.name = prop->name;
			pr.desc = prop->desc;
			vec.iov_base = buf;
			vec.iov_len  = sizeof(buf) - vec.iov_len;
			buf[vec.iov_len++] = 0;
			MPT_property_set_data(&pr, MPT_type_toVector('c'), &vec);
			return par->h(par->p, &pr);
		}
	}
	return par->h(par->p, prop);
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
