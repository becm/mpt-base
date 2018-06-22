/*!
 * set metatype data.
 */

#include "meta.h"

struct wrapIter
{
	MPT_INTERFACE(metatype) _ctl;
	MPT_INTERFACE(iterator) *it;
};

static void metaIterUnref(MPT_INTERFACE(reference) *ref)
{
	(void) ref;
}
static uintptr_t metaIterRef(MPT_INTERFACE(reference) *ref)
{
	(void) ref;
	return 0;
}
static MPT_INTERFACE(metatype) *metaIterClone(const MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
static int metaIterConv(const MPT_INTERFACE(metatype) *mt, int type, void *dest)
{
	const struct wrapIter *wr = (void *) mt;
	
	if (!type) {
		static const uint8_t fmt[] = { MPT_ENUM(TypeIterator), 0 };
		if (dest) {
			*((const uint8_t **) dest) = fmt;
			return 0;
		}
		return MPT_ENUM(TypeIterator);
	}
	if (type == MPT_ENUM(TypeMetaPtr)) {
		if (dest) *((const void **) dest) = &wr->_ctl;
		return MPT_ENUM(TypeIterator);
	}
	if (type == MPT_type_pointer(MPT_ENUM(TypeIterator))) {
		if (dest) *((const void **) dest) = wr->it;
		return MPT_ENUM(TypeIterator);
	}
	return MPT_ERROR(BadType);
}
