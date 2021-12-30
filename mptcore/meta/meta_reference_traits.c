/*!
 * register array with type system.
 */

#include "types.h"

#include "meta.h"

static void _meta_ref_fini(void *ptr)
{
	MPT_INTERFACE(metatype) *mt = *((MPT_INTERFACE(metatype) **) ptr);
	if (mt) {
		mt->_vptr->unref(mt);
	}
}
static int _meta_ref_init(void *ptr, const void *src)
{
	MPT_INTERFACE(metatype) *mt = 0;
	if (src && (mt = *((MPT_INTERFACE(metatype) * const *) src))) {
		if (!mt->_vptr->addref(mt)) {
			return MPT_ERROR(BadOperation);
		}
	}
	*((void **) ptr) = mt;
	return mt ? 1 : 0;
}

/*!
 * \ingroup mptMeta
 * \brief get array traits
 * 
 * Get array type operations and size.
 * 
 * \return array type traits
 */
extern const MPT_STRUCT(type_traits) *mpt_meta_reference_traits()
{
	static const MPT_STRUCT(type_traits) traits = {
		_meta_ref_init,
		_meta_ref_fini,
		sizeof(MPT_INTERFACE(metatype) *)
	};
	return &traits;
}
