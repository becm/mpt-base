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
		static const char fmt[] = { MPT_ENUM(TypeIterator), 0 };
		if (dest) *((const char **) dest) = fmt;
		return MPT_ENUM(TypeMeta);
	}
	if (type == MPT_ENUM(TypeMeta)) {
		if (dest) *((const void **) dest) = &wr->_ctl;
		return MPT_ENUM(TypeIterator);
	}
	if (type == MPT_ENUM(TypeIterator)) {
		if (dest) *((const void **) dest) = wr->it;
		return MPT_ENUM(TypeMeta);
	}
	return MPT_ERROR(BadType);
}