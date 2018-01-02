/*!
 * create values from grid points and polynom parameter
 */

#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <errno.h>

#include "convert.h"
#include "meta.h"

#include "values.h"

struct _iter_poly
{
	_MPT_ARRAY_TYPE(double) grid;
	long pos;
	int coeff;
};

/* reference interface */
static void iterPolyUnref(MPT_INTERFACE(reference) *ref)
{
	struct _iter_poly *d = (void *) (ref + 2);
	mpt_array_clone(&d->grid, 0);
	free(ref);
}
static uintptr_t iterPolyRef(MPT_INTERFACE(reference) *ref)
{
	(void) ref;
	return 0;
}
/* metatype interface */
static int iterPolyConv(const MPT_INTERFACE(metatype) *mt, int type, void *ptr)
{
	if (!type) {
		static const uint8_t fmt[] = { MPT_ENUM(TypeIterator) };
		if (ptr) {
			*((const uint8_t **) ptr) = fmt;
			return 'd';
		}
		return MPT_ENUM(TypeIterator);
	}
	if (type == MPT_ENUM(TypeIterator)) {
		if (ptr) *((const void **) ptr) = mt + 1;
		return MPT_ENUM(TypeIterator);
	}
	return MPT_ERROR(BadType);
}
static MPT_INTERFACE(metatype) *iterPolyClone(const MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
/* iterator interface */
static int iterPolyGet(MPT_INTERFACE(iterator) *it, int type, void *ptr)
{
	struct _iter_poly *d = (void *) (it + 1);
	MPT_STRUCT(buffer) *buf;
	struct {
		double shift, mult;
	} *coeff = (void *) (d + 1);
	double sum, val;
	long max, j;
	
	if (!type) {
		static const uint8_t fmt[] = "d";
		if (ptr) {
			*((const uint8_t **) ptr) = fmt;
		}
		return d->pos;
	}
	if (type != 'd') {
		return MPT_ERROR(BadType);
	}
	if ((buf = d->grid._buf)) {
		const double *src = (void *) (buf + 1);
		max = buf->_used / sizeof(double);
		if (d->pos >= max) {
			return 0;
		}
		val = src[d->pos];
	}
	else if (d->pos >= UINT_MAX) {
		return 0;
	}
	else {
		val = d->pos;
	}
	if (!ptr) {
		return 'd';
	}
	if (!(max = d->coeff)) {
		*((double *) ptr) = val;
		return 'd';
	}
	for (j = 0, sum = 0.0; j < max; j++) {
		double prod, tmp = val;
		int k;
		
		tmp += coeff[j].shift;
		
		for (k = j + 1, prod = coeff[j].mult; k < max; k++) {
			prod *= tmp;
		}
		sum += prod;
	}
	*((double *) ptr) = sum;
	
	return 'd';
}
static int iterPolyAdvance(MPT_INTERFACE(iterator) *it)
{
	struct _iter_poly *d = (void *) (it + 1);
	MPT_STRUCT(buffer) *buf;
	
	if ((buf = d->grid._buf)) {
		long max = buf->_used / sizeof(double);
		if (d->pos >= max) {
			return MPT_ERROR(BadOperation);
		}
		if (++d->pos == max) {
			return 0;
		}
		return 'd';
	}
	if (d->pos >= UINT_MAX) {
		return MPT_ERROR(BadOperation);
	}
	if (++d->pos == UINT_MAX) {
		return 0;
	}
	return 'd';
}
static int iterPolyReset(MPT_INTERFACE(iterator) *it)
{
	struct _iter_poly *d = (void *) (it + 1);
	MPT_STRUCT(buffer) *buf;
	d->pos = 0;
	if ((buf = d->grid._buf)) {
		return buf->_used / sizeof(double);
	}
	return 0;
}
/*!
 * \ingroup mptValues
 * \brief create polynom profile
 * 
 * Set data to result of polynom given according value from reference data.
 * 
 * \param points number of elements to set
 * \param target address of elements to set
 * \param ld     element advance
 * \param desc   polynam description
 * \param src    reference data
 * 
 * \return zero on success
 */
extern MPT_INTERFACE(metatype) *mpt_iterator_poly(const char *desc, const _MPT_ARRAY_TYPE(double) *base)
{
	static const MPT_INTERFACE_VPTR(metatype) polyMeta = {
		{ iterPolyUnref, iterPolyRef },
		iterPolyConv,
		iterPolyClone
	};
	static const MPT_INTERFACE_VPTR(iterator) polyIter = {
		iterPolyGet,
		iterPolyAdvance,
		iterPolyReset
	};
	MPT_INTERFACE(metatype) *mt;
	MPT_INTERFACE(iterator) *it;
	struct _iter_poly *d;
	struct {
		double shift, mult;
	} coeff[128];
	int nc = 0, ns = sizeof(coeff)/sizeof(*coeff);
	
	/* polynom coefficients */
	if (desc) {
		do {
			ssize_t len = mpt_cdouble(&coeff[nc].mult, desc, 0);
			
			if (len <= 0) {
				if (!nc) {
					errno = EINVAL;
					return 0;
				}
				break;
			}
			desc += len;
		} while (++nc < ns);
	}
	/* variable shift */
	ns = 0;
	if (nc && (desc = strchr(desc, ':'))) {
		int max = nc - 1;
		++desc;
		while (ns < max) {
			ssize_t len = mpt_cdouble(&coeff[ns].shift, desc, 0);
			
			if (len <= 0) {
				break;
			}
			desc += len;
			++ns;
		}
	}
	for ( ; ns < nc; ++ns) {
		coeff[ns].shift = 0;
	}
	if (!(mt = malloc(sizeof(*mt) + sizeof(*it) + sizeof(*d) + nc * sizeof(*coeff)))) {
		return 0;
	}
	mt->_vptr = &polyMeta;
	
	it = (void *) (mt + 1);
	it->_vptr = &polyIter;
	
	d = (void *) (it + 1);
	d->grid._buf = (base && base->_buf && mpt_refcount_raise(&base->_buf->_ref)) ? base->_buf : 0;
	d->pos = 0;
	d->coeff = nc;
	memcpy(d + 1, coeff, nc * sizeof(*coeff));
	
	return mt;
}
