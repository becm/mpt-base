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

MPT_STRUCT(iteratorBoundary)
{
	MPT_INTERFACE(metatype) _mt;
	MPT_INTERFACE(iterator) _it;
	MPT_STRUCT(value) val;
	
	double   left,  /* left boundary value */
	         inter, /* non-bounday value */
	         right; /* right boundary value */
	uint32_t elem,  /* number of elements */
	         pos;
};

/* convertable interface */
static int iterBoundaryConv(MPT_INTERFACE(convertable) *val, int type, void *ptr)
{
	MPT_STRUCT(iteratorBoundary) *d = MPT_baseaddr(iteratorBoundary, val, _mt);
	if (!type) {
		static const uint8_t fmt[] = { MPT_ENUM(TypeIteratorPtr), 0 };
		if (ptr) {
			*((const uint8_t **) ptr) = fmt;
			return 'd';
		}
		return MPT_ENUM(TypeIteratorPtr);
	}
	if (type == MPT_ENUM(TypeIteratorPtr)) {
		if (ptr) *((void **) ptr) = (void *) (&d->_it);
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
	MPT_STRUCT(iteratorBoundary) *d = MPT_baseaddr(iteratorBoundary, mt, _mt);
	MPT_INTERFACE(metatype) *ptr;
	
	if ((ptr = mpt_iterator_boundary(d->elem, d->left, d->inter, d->right))) {
		uint32_t pos = d->pos;
		d = (void *) (ptr + 2);
		d->pos = pos;
	}
	return ptr;
}
/* iterator interface */
static const MPT_STRUCT(value) *iterBoundaryValue(MPT_INTERFACE(iterator) *it)
{
	MPT_STRUCT(iteratorBoundary) *d = MPT_baseaddr(iteratorBoundary, it, _it);
	const double *val;
	
	if (d->pos >= d->elem) {
		return 0;
	}
	if (!d->pos) {
		val = &d->left;
	}
	else if (d->pos < (d->elem - 1)) {
		val = &d->inter;
	}
	else {
		val = &d->right;
	}
	d->val.domain = 0;
	d->val.type = 'd';
	d->val.ptr = memcpy (d->val._buf, val, sizeof(*val));
	
	return &d->val;
}
static int iterBoundaryAdvance(MPT_INTERFACE(iterator) *it)
{
	MPT_STRUCT(iteratorBoundary) *d = MPT_baseaddr(iteratorBoundary, it, _it);
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
	MPT_STRUCT(iteratorBoundary) *d = MPT_baseaddr(iteratorBoundary, it, _it);
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
		iterBoundaryValue,
		iterBoundaryAdvance,
		iterBoundaryReset
	};
	MPT_STRUCT(iteratorBoundary) *data;
	if (len < 2) {
		errno = EINVAL;
		return 0;
	}
	if (!(data = malloc(sizeof(*data)))) {
		return 0;
	}
	data->_mt._vptr = &boundaryMeta;
	data->_it._vptr = &boundaryIter;
	
	data->val.domain = 0;
	data->val.type = 0;
	*((uint8_t *) &data->val._bufsize) = sizeof(data->val._buf);
	
	data->left = left;
	data->inter = inter;
	data->right = right;
	data->elem = len;
	data->pos  = 0;
	
	return &data->_mt;
}

