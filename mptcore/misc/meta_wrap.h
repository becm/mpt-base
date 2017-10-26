/*!
 * set metatype data.
 */

#include "meta.h"

#include "object.h"

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
	struct wrapIter *wr = (void *) mt;
	MPT_INTERFACE(iterator) *it;
	
	if (!type) {
		static const char fmt[] = { MPT_ENUM(TypeIterator), MPT_ENUM(TypeValue), 0 };
		if (dest) {
			*((const char **) dest) = fmt;
		}
		return *fmt;
	}
	if (type == MPT_ENUM(TypeIterator)) {
		if (dest) {
			*((const void **) dest) = wr->it;
		}
		return MPT_ENUM(TypeValue);
	}
	if (!(it = wr->it)) {
		return MPT_ERROR(BadType);
	}
	return it->_vptr->get(it, type, dest);
}
static const MPT_INTERFACE_VPTR(metatype) metaIterCtl = {
	{ metaIterUnref, metaIterRef },
	metaIterConv,
	metaIterClone
};
