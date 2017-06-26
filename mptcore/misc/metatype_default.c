/*!
 * basic metatype
 */

#include <errno.h>
#include <stdlib.h>

#include "meta.h"

static void genMetaUnref(MPT_INTERFACE(unrefable) *meta)
{
	(void) meta;
}
static int genMetaConv(const MPT_INTERFACE(metatype) *meta, int type, void *ptr)
{
	void **dest = ptr;
	
	if (!type) {
		static const char types[] = { MPT_ENUM(TypeMeta), 's', 0 };
		if (dest) *dest = (void *) types;
		return 0;
	}
	if (type == MPT_ENUM(TypeMeta)) {
		if (dest) {
			*dest = (void *) meta;
		}
		return 0;
	}
	return MPT_ERROR(BadType);
}
static MPT_INTERFACE(metatype) *genMetaClone(const MPT_INTERFACE(metatype) *meta)
{
	return (void *) meta;
}

/*!
 * \ingroup mptMeta
 * \brief generic default metatype
 * 
 * Get shared instance for default metatype value.
 * 
 * \return new basic metatype instance
 */
extern MPT_INTERFACE(metatype) *mpt_metatype_default()
{
	static const MPT_INTERFACE_VPTR(metatype) ctl = {
		{ genMetaUnref },
		genMetaConv,
		genMetaClone
	};
	static MPT_INTERFACE(metatype) mt = { &ctl };
	return &mt;
}
