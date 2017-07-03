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
	double   left,  /* current/base value */
	         inter,  /* advance step size */
	         right;  /* advance step size */
	uint32_t elem,  /* number of elements */
	         pos;
};

static void iterBoundaryUnref(MPT_INTERFACE(unrefable) *ref)
{
	free(ref);
}
static int iterBoundaryGet(MPT_INTERFACE(iterator) *it, int t, void *ptr)
{
	struct _iter_bdata *d = (void *) (it + 1);
	if (!t) {
		MPT_STRUCT(value) *val;
		static const char fmt[] = { 'd', 'd', 'd', 'u', 0 };
		if ((val = ptr)) {
			val->fmt = fmt;
			val->ptr = d;
		}
		return d->pos;
	}
	if (d->pos >= d->elem) {
		return MPT_ERROR(MissingData);
	}
	if (t == 'd') {
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
	if (t == 'f') {
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
static const MPT_INTERFACE_VPTR(iterator) iteratorBoundary = {
	{ iterBoundaryUnref },
	iterBoundaryGet,
	iterBoundaryAdvance,
	iterBoundaryReset
};

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
extern MPT_INTERFACE(iterator) *mpt_iterator_boundary(uint32_t len, double left, double inter, double right)
{
	MPT_INTERFACE(iterator) *iter;
	struct _iter_bdata *data;
	if (len < 2) {
		errno = EINVAL;
		return 0;
	}
	if (!(iter = malloc(sizeof(*iter) + sizeof(*data)))) {
		return 0;
	}
	iter->_vptr = &iteratorBoundary;
	data = (void *) (iter + 1);
	
	data->left = left;
	data->inter = inter;
	data->right = right;
	data->elem = len;
	data->pos  = 0;
	
	return iter;
}

