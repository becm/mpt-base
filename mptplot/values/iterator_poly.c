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

struct iterPoly
{
	MPT_INTERFACE(iterator) _it;
	_MPT_ARRAY_TYPE(double) grid;
	long pos;
	int coeff;
};

static void iterPolyUnref(MPT_INTERFACE(reference) *ref)
{
	struct iterPoly *ip = (void *) ref;
	mpt_array_clone(&ip->grid, 0);
	free(ref);
}
static uintptr_t iterPolyRef(MPT_INTERFACE(reference) *ref)
{
	(void) ref;
	return 0;
}
static int iterPolyGet(MPT_INTERFACE(iterator) *it, int type, void *dest)
{
	struct iterPoly *p = (void *) it;
	MPT_STRUCT(buffer) *buf;
	struct {
		double shift, mult;
	} *coeff = (void *) (p + 1);
	double sum, val;
	long max, j;
	
	if (type != 'd') {
		return MPT_ERROR(BadType);
	}
	if ((buf = p->grid._buf)) {
		const double *src = (void *) (buf + 1);
		max = buf->_used / sizeof(double);
		if (p->pos >= max) {
			return 0;
		}
		val = src[p->pos];
	}
	else if (p->pos >= UINT_MAX) {
		return 0;
	}
	else {
		val = p->pos;
	}
	if (!(max = p->coeff)) {
		if (dest) {
			*((double *) dest) = val;
		}
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
	if (dest) {
		*((double *) dest) = sum;
	}
	return 'd';
}
static int iterPolyAdvance(MPT_INTERFACE(iterator) *it)
{
	struct iterPoly *ip = (void *) it;
	MPT_STRUCT(buffer) *buf;
	if ((buf = ip->grid._buf)) {
		long max = buf->_used / sizeof(double);
		if (ip->pos >= max) {
			return MPT_ERROR(BadOperation);
		}
		if (++ip->pos == max) {
			return 0;
		}
		return 'd';
	}
	if (ip->pos >= UINT_MAX) {
		return MPT_ERROR(BadOperation);
	}
	if (++ip->pos == UINT_MAX) {
		return 0;
	}
	return 'd';
}
static int iterPolyReset(MPT_INTERFACE(iterator) *it)
{
	struct iterPoly *ip = (void *) it;
	MPT_STRUCT(buffer) *buf;
	ip->pos = 0;
	if ((buf = ip->grid._buf)) {
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
extern MPT_INTERFACE(iterator) *mpt_iterator_poly(const char *desc, const _MPT_ARRAY_TYPE(double) *base)
{
	static const MPT_INTERFACE_VPTR(iterator) polyIterCtl = {
		{ iterPolyUnref, iterPolyRef },
		iterPolyGet,
		iterPolyAdvance,
		iterPolyReset
	};
	struct iterPoly *ip;
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
	if (!(ip = malloc(sizeof(*ip) + nc * sizeof(*coeff)))) {
		return 0;
	}
	ip->_it._vptr = &polyIterCtl;
	ip->grid._buf = (base && base->_buf && mpt_refcount_raise(&base->_buf->_ref)) ? base->_buf : 0;
	ip->pos = 0;
	ip->coeff = nc;
	memcpy(ip + 1, coeff, nc * sizeof(*coeff));
	
	return &ip->_it;
}

