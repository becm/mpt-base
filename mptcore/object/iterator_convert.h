/*!
 * wrap iterator in convertable.
 */

#include "core.h"

struct convIter
{
	MPT_INTERFACE(convertable) _ctl;
	MPT_INTERFACE(iterator) *it;
};

static int iteratorConv(MPT_INTERFACE(convertable) *mt, int type, void *dest)
{
	const struct convIter *wr = (void *) mt;
	
	if (!type) {
		static const uint8_t fmt[] = { MPT_ENUM(TypeIterator), 0 };
		if (dest) {
			*((const uint8_t **) dest) = fmt;
			return 0;
		}
		return MPT_ENUM(TypeIterator);
	}
	if (type == MPT_type_pointer(MPT_ENUM(TypeIterator))) {
		if (dest) *((const void **) dest) = wr->it;
		return MPT_ENUM(TypeIterator);
	}
	return MPT_ERROR(BadType);
}
