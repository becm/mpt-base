/*!
 * MPT core library
 *   traits for config item
 */

#include "array.h"
#include "meta.h"
#include "types.h"

#include "config.h"

static int _init_config_item(void *addr, const void *base)
{
	MPT_STRUCT(config_item) *dst = addr;
	const MPT_STRUCT(config_item) *src;
	dst->elements._buf = 0;
	dst->value = 0;
	mpt_identifier_init(&dst->identifier, sizeof(dst->identifier));
	if ((src = base)) {
		MPT_INTERFACE(metatype) *m;
		mpt_identifier_copy(&dst->identifier, &src->identifier);
		if ((m = src->value) && m->_vptr->addref(m)) {
			dst->value = m;
		}
	}
	return 0;
}

static void _fini_config_item(void *addr) {
	MPT_STRUCT(config_item) *item = addr;
	MPT_INTERFACE(metatype) *m;
	
	mpt_array_clone(&item->elements, 0);
	if ((m = item->value)) {
		m->_vptr->unref(m);
		item->value = 0;
	}
	mpt_identifier_set(&item->identifier, 0, 0);
}

/*!
 * \ingroup mptConfig
 * \brief config item traits
 * 
 * Get type traits for configuration item for path.
 * 
 * \return type traits for configuration item
 */
extern const MPT_STRUCT(type_traits) *mpt_config_item_traits()
{
	static const MPT_STRUCT(type_traits) traits = {
		_init_config_item,
		_fini_config_item,
		sizeof(MPT_STRUCT(config_item))
	};
	return &traits;
}
