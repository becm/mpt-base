/*!
 * default metatype
 */

#include <errno.h>
#include <stdlib.h>

#include <sys/uio.h>

#include "types.h"

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
	MPT_STRUCT(value) val;
	struct iovec vec;
	
	if (_mpt_geninfo_conv(info, MPT_type_toVector('c'), &vec) < 0) {
		errno = EINVAL;
		return 0;
	}
	MPT_value_set_data(&val, MPT_type_toVector('c'), &vec);
	return mpt_meta_new(&val);
}
