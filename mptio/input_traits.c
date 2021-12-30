/*!
 * register input with type system.
 */

#include "types.h"

#include "notify.h"

static int _input_ref_init(void *ptr, const void *src)
{
	MPT_INTERFACE(metatype) *const *from;
	MPT_INTERFACE(metatype) *meta = 0;
	intptr_t ret = 0;
	
	if ((from = src)
	    && (meta = *from)
	    && ((ret = meta->_vptr->addref(meta)) < 0)) {
		return MPT_ERROR(BadOperation);
	}
	*((MPT_INTERFACE(metatype) **) ptr) = meta;
	    
	return ret ? 1 : 0;
}
static void _input_ref_fini(void *ptr)
{
	MPT_INTERFACE(metatype) **ref_ptr = ptr, *ref;
	
	if ((ref = *ref_ptr)) {
		ref->_vptr->unref(ref);
		*ref_ptr = 0;
	}
}

/*!
 * \ingroup mptNotify
 * \brief get input reference traits
 * 
 * Get input reference type operations and size.
 * 
 * \return input reference type traits
 */
extern const MPT_STRUCT(type_traits) *mpt_input_reference_traits()
{
	static const MPT_STRUCT(type_traits) traits = {
		_input_ref_init,
		_input_ref_fini,
		sizeof(MPT_INTERFACE(metatype) *)
	};
	return &traits;
}

/*!
 * \ingroup mptNotify
 * \brief get input pointer traits
 * 
 * Get or register input metatype pointer.
 * 
 * \return input id
 */
extern const MPT_STRUCT(named_traits) *mpt_input_type_traits(void)
{
	static const MPT_STRUCT(named_traits) *traits = 0;
	
	if (!traits) {
		traits = mpt_type_metatype_add("mpt.input");
	}
	return traits;
}
