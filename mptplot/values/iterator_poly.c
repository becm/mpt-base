/*!
 * create values from grid points and polynom parameter
 */

#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <errno.h>

#include "convert.h"
#include "types.h"
#include "meta.h"

#include "values.h"

struct coeff_poly {
	double shift, mult;
};

MPT_STRUCT(iteratorPolynom)
{
	MPT_INTERFACE(metatype) _mt;
	MPT_INTERFACE(iterator) _it;
	
	MPT_STRUCT(value) val;
	double curr;
	
	_MPT_ARRAY_TYPE(double) grid;
	long pos;
	int coeff;
};

/* convertable interface */
static int iterPolyConv(MPT_INTERFACE(convertable) *val, int type, void *ptr)
{
	MPT_STRUCT(iteratorPolynom) *d = MPT_baseaddr(iteratorPolynom, val, _mt);
	if (!type) {
		static const uint8_t fmt[] = { MPT_ENUM(TypeIteratorPtr), 0 };
		if (ptr) {
			*((const uint8_t **) ptr) = fmt;
			return 'd';
		}
		return MPT_ENUM(TypeIteratorPtr);
	}
	if (type == MPT_ENUM(TypeIteratorPtr)) {
		if (ptr) *((const void **) ptr) = &d->_it;
		return MPT_ENUM(TypeIteratorPtr);
	}
	return MPT_ERROR(BadType);
}
/* metatype interface */
static void iterPolyUnref(MPT_INTERFACE(metatype) *mt)
{
	MPT_STRUCT(iteratorPolynom) *d = MPT_baseaddr(iteratorPolynom, mt, _mt);
	mpt_array_clone(&d->grid, 0);
	free(mt);
}
static uintptr_t iterPolyRef(MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
static MPT_INTERFACE(metatype) *iterPolyClone(const MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
/* iterator interface */
static const MPT_STRUCT(value) *iterPolyValue(MPT_INTERFACE(iterator) *it)
{
	MPT_STRUCT(iteratorPolynom) *d = MPT_baseaddr(iteratorPolynom, it, _it);
	MPT_STRUCT(buffer) *buf;
	double sum, val;
	long max, j;
	
	/* check limits and set initial value */
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
	/* reuse calculations of previous call */
	if (d->val._type) {
		return &d->val;
	}
	/* no coefficients */
	if (!(max = d->coeff)) {
		sum = val;
	}
	/* calculate polynom */
	else {
		const struct coeff_poly *coeff = (void *) (d + 1);
		for (j = 0, sum = 0.0; j < max; j++) {
			double prod, tmp = val;
			int k;
			
			tmp += coeff[j].shift;
			
			for (k = j + 1, prod = coeff[j].mult; k < max; k++) {
				prod *= tmp;
			}
			sum += prod;
		}
	}
	/* assign new value */
	d->curr = sum;
	MPT_value_set(&d->val, 'd', &d->curr);
	
	return &d->val;
}
static int iterPolyAdvance(MPT_INTERFACE(iterator) *it)
{
	MPT_STRUCT(iteratorPolynom) *d = MPT_baseaddr(iteratorPolynom, it, _it);
	const MPT_STRUCT(buffer) *buf;
	
	if ((buf = d->grid._buf)) {
		long max = buf->_used / sizeof(double);
		if (d->pos >= max) {
			return MPT_ERROR(BadOperation);
		}
		d->val._type = 0;
		if (++d->pos == max) {
			return 0;
		}
		return 'd';
	}
	if (d->pos >= UINT_MAX) {
		return MPT_ERROR(MissingData);
	}
	d->val._type = 0;
	if (++d->pos == UINT_MAX) {
		return 0;
	}
	return 'd';
}
static int iterPolyReset(MPT_INTERFACE(iterator) *it)
{
	MPT_STRUCT(iteratorPolynom) *d = MPT_baseaddr(iteratorPolynom, it, _it);
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
		{ iterPolyConv },
		iterPolyUnref,
		iterPolyRef,
		iterPolyClone
	};
	static const MPT_INTERFACE_VPTR(iterator) polyIter = {
		iterPolyValue,
		iterPolyAdvance,
		iterPolyReset
	};
	MPT_STRUCT(iteratorPolynom) *data;
	MPT_STRUCT(buffer) *buf;
	struct coeff_poly coeff[128];
	int nc = 0, ns = sizeof(coeff)/sizeof(*coeff);
	
	if ((buf = base->_buf)
	    && !buf->_vptr->addref(buf)) {
		return 0;
	}
	/* polynom coefficients */
	if (desc) {
		do {
			ssize_t len = mpt_cdouble(&coeff[nc].mult, desc, 0);
			
			if (len <= 0) {
				if (!nc) {
					if (buf) {
						buf->_vptr->unref(buf);
					}
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
	if (!(data = malloc(sizeof(*data) + nc * sizeof(*coeff)))) {
		if (buf) {
			buf->_vptr->unref(buf);
		}
		return 0;
	}
	data->_mt._vptr = &polyMeta;
	data->_it._vptr = &polyIter;
	
	MPT_value_set(&data->val, 0, 0);
	
	data->grid._buf = buf;
	data->pos = 0;
	data->coeff = nc;
	memcpy(data + 1, coeff, nc * sizeof(*coeff));
	
	return &data->_mt;
}
