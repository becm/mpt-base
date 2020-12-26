/*!
 * create iterator with constant difference between elements.
 */

#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <errno.h>

#include "convert.h"
#include "types.h"
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

/* convertable interface */
static int iterBoundaryConv(MPT_INTERFACE(convertable) *val, int type, void *ptr)
{
	if (!type) {
		static const uint8_t fmt[] = { MPT_ENUM(TypeIteratorPtr), 0 };
		if (ptr) {
			*((const uint8_t **) ptr) = fmt;
			return 'd';
		}
		return MPT_ENUM(TypeIteratorPtr);
	}
	if (type == MPT_ENUM(TypeIteratorPtr)) {
		if (ptr) *((void **) ptr) = (void *) (val + 1);
		return 'd';
	}
	return MPT_ERROR(BadType);
}
/* metatype interface */
static void iterBoundaryUnref(MPT_INTERFACE(metatype) *mt)
{
	free(mt);
}
static uintptr_t iterBoundaryRef(MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
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
	double val;
	
	if (!type) {
		static const uint8_t fmt[] = "df";
		if (ptr) {
			*((const uint8_t **) ptr) = fmt;
		}
		return d->pos;
	}
	if (d->pos > d->elem) {
		return MPT_ERROR(MissingData);
	}
	if (d->pos == d->elem) {
		return 0;
	}
	if (!d->pos) {
		val = d->left;
	}
	else if (d->pos < (d->elem - 1)) {
		val = d->inter;
	}
	else {
		val = d->right;
	}
	if (type == 'd') {
		if (ptr) *((double *) ptr) = val;
		return 'd';
	}
	if (type == 'f') {
		if (ptr) *((float *) ptr) = val;
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
		{ iterBoundaryConv },
		iterBoundaryUnref,
		iterBoundaryRef,
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

