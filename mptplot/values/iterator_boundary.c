/*!
 * create iterator with constant difference between elements.
 */

#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <errno.h>

#include "convert.h"
#include "meta.h"
#include "parse.h"

#include "values.h"

struct _iter_bdata
{
	double   left,  /* left boundary value */
	         inter, /* non-bounday value */
	         right; /* right boundary value */
	uint32_t elem,  /* number of elements */
	         pos;
};

/* reference interface */
static void iterBoundaryUnref(MPT_INTERFACE(reference) *ref)
{
	free(ref);
}
static uintptr_t iterBoundaryRef(MPT_INTERFACE(reference) *ref)
{
	(void) ref;
	return 0;
}
/* metatype interface */
static int iterBoundaryConv(const MPT_INTERFACE(metatype) *mt, int type, void *ptr)
{
	if (!type) {
		static const char fmt[] = { MPT_ENUM(TypeIterator), 0 };
		if (ptr) *((const char **) ptr) = fmt;
		return MPT_ENUM(TypeIterator);
	}
	if (type == MPT_ENUM(TypeIterator)) {
		if (ptr) *((void **) ptr) = (void *) (mt + 1);
		return MPT_ENUM(TypeIterator);
	}
	if (type == MPT_ENUM(TypeValue)) {
		MPT_STRUCT(value) *val;
		if ((val = ptr)) {
			static const char fmt[] = { 'd', 'd', 'd', 'u', 0 };
			val->fmt = fmt;
			val->ptr = (void *) (mt + 2);
		}
		return MPT_ENUM(TypeIterator);
	}
	return MPT_ERROR(BadType);
}
static MPT_INTERFACE(metatype) *iterBoundaryClone(const MPT_INTERFACE(metatype) *mt)
{
	struct _iter_bdata *d = (void *) (mt + 2);
	MPT_INTERFACE(metatype) *ptr;
	
	if ((ptr = mpt_iterator_boundary(d->elem, d->left, d->inter, d->right))) {
		uint32_t pos = d->pos;
		d = (void *) (ptr + 2);
		d->pos = pos;
	}
	return ptr;
}
/* iterator interface */
static int iterBoundaryGet(MPT_INTERFACE(iterator) *it, int type, void *ptr)
{
	struct _iter_bdata *d = (void *) (it + 1);
	if (!type) {
		static const char fmt[] = { 'd', 'f', 0 };
		if (ptr) *((const char **) ptr) = fmt;
		return d->pos;
	}
	if (d->pos >= d->elem) {
		return MPT_ERROR(MissingData);
	}
	if (type == 'd') {
		if (ptr) {
			if (!d->pos) {
				*((double *) ptr) = d->left;
			}
			else if (d->pos < (d->elem - 1)) {
				*((double *) ptr) = d->inter;
			} else {
				*((double *) ptr) = d->right;
			}
		}
		return 'd';
	}
	if (type == 'f') {
		if (ptr) {
			if (!d->pos) {
				*((float *) ptr) = d->left;
			}
			else if (d->pos < (d->elem - 1)) {
				*((float *) ptr) = d->inter;
			} else {
				*((float *) ptr) = d->right;
			}
		}
		return 'd';
	}
	return MPT_ERROR(BadType);
}
static int iterBoundaryAdvance(MPT_INTERFACE(iterator) *it)
{
	struct _iter_bdata *d = (void *) (it + 1);
	if (d->pos >= d->elem) {
		return MPT_ERROR(MissingData);
	}
	if (++d->pos == d->elem) {
		return 0;
	}
	return 'd';
}
static int iterBoundaryReset(MPT_INTERFACE(iterator) *it)
{
	struct _iter_bdata *d = (void *) (it + 1);
	d->pos = 0;
	return d->elem;
}

/*!
 * \ingroup mptValues
 * \brief create linear iterator
 * 
 * Create iterator with linear advancing.
 * 
 * \param len    number of elements to serve
 * \param start  start value
 * \param end    final value
 * 
 * \return linear iterator
 */
extern MPT_INTERFACE(metatype) *mpt_iterator_boundary(uint32_t len, double left, double inter, double right)
{
	static const MPT_INTERFACE_VPTR(metatype) boundaryMeta = {
		{ iterBoundaryUnref, iterBoundaryRef },
		iterBoundaryConv,
		iterBoundaryClone
	};
	static const MPT_INTERFACE_VPTR(iterator) boundaryIter = {
		iterBoundaryGet,
		iterBoundaryAdvance,
		iterBoundaryReset
	};
	MPT_INTERFACE(metatype) *mt;
	MPT_INTERFACE(iterator) *it;
	struct _iter_bdata *data;
	if (len < 2) {
		errno = EINVAL;
		return 0;
	}
	if (!(mt = malloc(sizeof(*mt) + sizeof(*it) + sizeof(*data)))) {
		return 0;
	}
	mt->_vptr = &boundaryMeta;
	
	it = (void *) (mt + 1);
	it->_vptr = &boundaryIter;
	
	data = (void *) (it + 1);
	data->left = left;
	data->inter = inter;
	data->right = right;
	data->elem = len;
	data->pos  = 0;
	
	return mt;
}

