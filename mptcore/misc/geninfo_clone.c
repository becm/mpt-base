/*!
 * default metatype
 */

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
extern MPT_INTERFACE(metatype) *_mpt_geninfo_clone(const uint64_t *info)
{
	static const char vecfmt[] = { 'c' - 0x40, 0 };
	MPT_INTERFACE(metatype) *meta;
	MPT_STRUCT(value) val;
	struct iovec vec;
	int len;
	
	len = _mpt_geninfo_value((uint64_t *) info, 0);
	
	if (!(meta = mpt_meta_new(len + 1))) {
		return 0;
	}
	vec.iov_len = len;
	vec.iov_base = (void *) (info + 1);
	val.fmt = vecfmt;
	val.ptr = &vec;
	if (meta->_vptr->assign(meta, &val) >= 0) {
		return meta;
	}
	meta->_vptr->ref.unref((void *) meta);
	return 0;
}
