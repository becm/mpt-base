/*!
 * basic metatype
 */

#include <errno.h>
#include <stdlib.h>

#include "meta.h"

static int genMetaConv(MPT_INTERFACE(convertable) *meta, int type, void *ptr)
{
	void **dest = ptr;
	
	if (!type) {
		static const uint8_t types[] = { 0 };
		if (dest) *dest = (void *) types;
		return 0;
	}
	if (type == MPT_ENUM(TypeMetaPtr)) {
		if (dest) *dest = (void *) meta;
		return 0;
	}
	return MPT_ERROR(BadType);
}
static void genMetaUnref(MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
}
static uintptr_t genMetaRef(MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 1;
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
		{ genMetaConv },
		genMetaUnref,
		genMetaRef,
		genMetaClone
	};
	static MPT_INTERFACE(metatype) mt = { &ctl };
	return &mt;
}
