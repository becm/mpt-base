/*!
 * default metatype
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

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
	struct iovec vec;
	MPT_STRUCT(value) val = MPT_VALUE_INIT(MPT_type_toVector('c'), &vec);
	
	if (_mpt_geninfo_conv(info, val._type, &vec) < 0) {
		errno = EINVAL;
		return 0;
	}
	return mpt_meta_new(&val);
}
