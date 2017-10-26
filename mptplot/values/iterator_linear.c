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

struct _iter_ldata
{
	double   base,  /* current/base value */
	         step;  /* advance step size */
	uint32_t elem,  /* number of elements */
	         pos;
};

static void iterUnref(MPT_INTERFACE(reference) *ref)
{
	free(ref);
}
static uintptr_t iterRef(MPT_INTERFACE(reference) *ref)
{
	(void) ref;
	return 0;
}
static int iterGet(MPT_INTERFACE(iterator) *it, int t, void *ptr)
{
	struct _iter_ldata *d = (void *) (it + 1);
	if (!t) {
		MPT_STRUCT(value) *val;
		static const char fmt[] = { 'd', 'd', 'u', 0 };
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
			*((double *) ptr) = d->base + d->pos * d->step;
		}
		return 'd';
	}
	if (t == 'f') {
		if (ptr) {
			*((float *) ptr) = d->base + d->pos * d->step;
		}
		return 'd';
	}
	return MPT_ERROR(BadType);
}
static int iterAdvance(MPT_INTERFACE(iterator) *it)
{
	struct _iter_ldata *d = (void *) (it + 1);
	if (d->pos >= d->elem) {
		return MPT_ERROR(MissingData);
	}
	if (++d->pos == d->elem) {
		return 0;
	}
	return 'd';
}
static int iterReset(MPT_INTERFACE(iterator) *it)
{
	struct _iter_ldata *d = (void *) (it + 1);
	d->pos = 0;
	return d->elem;
}
static const MPT_INTERFACE_VPTR(iterator) iteratorLinear = {
	{ iterUnref, iterRef },
	iterGet,
	iterAdvance,
	iterReset
};

static int parseRange(const char *from, MPT_STRUCT(range) *r)
{
	double tmp;
	int r1, r2;
	if ((r1 = mpt_cdouble(&tmp, from, 0)) < 0) {
		return r1;
	}
	if ((r2 = mpt_cdouble(&r->max, from + r1, 0)) < 0) {
		return r2;
	}
	r->min = tmp;
	return r1 + r2;
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
extern MPT_INTERFACE(iterator) *mpt_iterator_linear(uint32_t len, double start, double end)
{
	MPT_INTERFACE(iterator) *iter;
	struct _iter_ldata *data;
	if (len < 2) {
		errno = EINVAL;
		return 0;
	}
	if (!(iter = malloc(sizeof(*iter) + sizeof(*data)))) {
		return 0;
	}
	iter->_vptr = &iteratorLinear;
	data = (void *) (iter + 1);
	
	data->base = start;
	data->step = (end - start) / (len - 1);
	data->elem = len;
	data->pos  = 0;
	
	return iter;
}

/*!
 * \ingroup mptValues
 * \brief create linear iterator
 * 
 * Create iterator with linear advancing.
 * 
 * \param val  linear iterator parameters
 * 
 * \return linear iterator
 */
extern MPT_INTERFACE(iterator) *_mpt_iterator_linear(MPT_STRUCT(value) *val)
{
	MPT_STRUCT(range) r = { 0.0, 1.0 };
	uint32_t iv = 10;
	int ret;
	
	if (!val) {
		iv = 10;
	}
	else {
		const char *str;
		if ((str = val->fmt)) {
			if ((ret = mpt_value_read(val, "u", &iv)) < 1
			 || (ret = mpt_range_set(&r, val)) < 0) {
				errno = EINVAL;
				return 0;
			}
		}
		else if ((str = val->ptr)) {
			if ((ret = mpt_string_nextvis(&str)) < 0
			 || (ret != '(')
			 || (ret = mpt_cuint32(&iv, str + 1, 0, 0)) < 1) {
				errno = EINVAL;
				return 0;
			}
			str += ret + 1;
			if ((ret = mpt_string_nextvis(&str)) == ':') {
				if ((ret = parseRange(str + 1, &r)) < 0) {
					errno = EINVAL;
					return 0;
				}
				str += ret + 1;
			}
			if ((ret = mpt_string_nextvis(&str)) != ')') {
				errno = EINVAL;
				return 0;
			}
		}
	}
	return mpt_iterator_linear(iv + 1, r.min, r.max);
}

/*!
 * \ingroup mptValues
 * \brief create range iterator
 * 
 * Create iterator with linear advancing.
 * 
 * \param val  range iterator parameters
 * 
 * \return range iterator
 */
extern MPT_INTERFACE(iterator) *_mpt_iterator_range(MPT_STRUCT(value) *val)
{
	MPT_INTERFACE(iterator) *iter;
	struct _iter_ldata *data;
	MPT_STRUCT(range) r = { 0.0, 1.0 };
	double step = 0.1;
	int iv = 10;
	
	if (val) {
		const char *str;
		int ret;
		if ((str = val->fmt)) {
			if ((ret = mpt_range_set(&r, val)) < 0) {
				errno = EINVAL;
				return 0;
			}
			step = (r.max - r.min) / 10;
			if ((ret = mpt_value_read(val, "d", &step)) < 0) {
				errno = EINVAL;
				return 0;
			}
		}
		else if ((str = val->ptr) && *str) {
			if ((ret = mpt_string_nextvis(&str)) < 0
			 || (ret != '(')) {
				errno = EINVAL;
				return 0;
			}
			if ((ret = parseRange(str + 1, &r)) < 0) {
				errno = EINVAL;
				return 0;
			}
			str += ret + 1;
			step = (r.max - r.min) / 10;
			if ((ret = mpt_string_nextvis(&str)) == ':') {
				if ((ret = mpt_cdouble(&step, str + 1, 0)) < 0) {
					errno = EINVAL;
					return 0;
				}
				str += ret + 1;
			}
			if ((ret = mpt_string_nextvis(&str)) != ')') {
				errno = EINVAL;
				return 0;
			}
		}
		if (step > (r.max - r.min)
		  || step < (r.max - r.min) * 1e-6) {
			errno = ERANGE;
			return 0;
		}
		iv = (r.max - r.min) / step;
	}
	if (!(iter = malloc(sizeof(*iter) + sizeof(*data)))) {
		return 0;
	}
	iter->_vptr = &iteratorLinear;
	data = (void *) (iter + 1);
	
	data->base = r.min;
	data->step = step;
	data->elem = iv + 1;
	data->pos = 0;
	
	return iter;
}

