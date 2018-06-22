/*!
 * default metatype
 */

#include <errno.h>
#include <stdlib.h>

#include <sys/uio.h>

#include "meta.h"

/*!
 * \ingroup mptMeta
 * \brief generic metatype clone
 * 
 * Create clone of geninfo data in new metatype instance.
 * 
 * \param info  start of (meta)data for geninfo
 * 
 * \return new metatype instance
 */
extern MPT_INTERFACE(metatype) *_mpt_geninfo_clone(const void *info)
{
	static const uint8_t vecfmt[] = { MPT_type_vector('c'), 0 };
	MPT_STRUCT(value) val;
	struct iovec vec;
	int len;
	
	if ((len = _mpt_geninfo_conv(info, *vecfmt, &vec)) < 0) {
		errno = EINVAL;
		return 0;
	}
	val.fmt = vecfmt;
	val.ptr = &vec;
	return mpt_meta_new(val);
}
