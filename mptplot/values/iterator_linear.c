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

struct _iter_ldata
{
	double   base,  /* current/base value */
	         step;  /* advance step size */
	uint32_t elem,  /* number of elements */
	         pos;
};

/* convertable interface */
static int iterConv(MPT_INTERFACE(convertable) *val, int type, void *ptr)
{
	if (!type) {
		static const uint8_t fmt[] = { MPT_ENUM(TypeIterator) };
		if (ptr) {
			*((const uint8_t **) ptr) = fmt;
			return 'd';
		}
		return MPT_ENUM(TypeIterator);
	}
	if (type == MPT_type_pointer(MPT_ENUM(TypeIterator))) {
		if (ptr) *((const void **) ptr) = val + 1;
		return 'd';
	}
	return MPT_ERROR(BadType);
}
/* metatype interface */
static void iterUnref(MPT_INTERFACE(metatype) *mt)
{
	free(mt);
}
static uintptr_t iterRef(MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
static MPT_INTERFACE(metatype) *iterClone(const MPT_INTERFACE(metatype) *mt)
{
	struct _iter_ldata *d = (void *) (mt + 2);
	MPT_INTERFACE(metatype) *ptr;
	
	if ((ptr = _mpt_iterator_range(0))) {
		struct _iter_ldata *next = (void *) (ptr + 2);
		*next = *d;
	}
	return ptr;
}
/* iterator interface */
static int iterGet(MPT_INTERFACE(iterator) *it, int type, void *ptr)
{
	struct _iter_ldata *d = (void *) (it + 1);
	double val;
	
	if (!type) {
		static const uint8_t fmt[] = "df";
		if (ptr) {
			*((const uint8_t **) ptr) = fmt;
		}
		return d->pos;
	}
	if (d->pos >= d->elem) {
		return MPT_ERROR(MissingData);
	}
	val = d->base + d->pos * d->step;
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
static const MPT_INTERFACE_VPTR(metatype) _vptr_linear_meta = {
	{ iterConv },
	iterUnref,
	iterRef,
	iterClone
};
static const MPT_INTERFACE_VPTR(iterator) _vptr_linear_iter = {
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
extern MPT_INTERFACE(metatype) *mpt_iterator_linear(uint32_t len, double start, double end)
{
	MPT_INTERFACE(metatype) *mt;
	MPT_INTERFACE(iterator) *it;
	struct _iter_ldata *d;
	if (len < 2) {
		errno = EINVAL;
		return 0;
	}
	if (!(mt = malloc(sizeof(*mt) + sizeof(*it) + sizeof(*d)))) {
		return 0;
	}
	mt->_vptr = &_vptr_linear_meta;
	
	it = (void *) (mt + 1);
	it->_vptr = &_vptr_linear_iter;
	
	d = (void *) (it + 1);
	d->base = start;
	d->step = (end - start) / (len - 1);
	d->elem = len;
	d->pos  = 0;
	
	return mt;
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
extern MPT_INTERFACE(metatype) *_mpt_iterator_linear(MPT_STRUCT(value) *val)
{
	MPT_STRUCT(range) r = { 0.0, 1.0 };
	uint32_t iv = 10;
	int ret;
	
	if (!val) {
		iv = 10;
	}
	else {
		const char *str;
		if (val->fmt) {
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
extern MPT_INTERFACE(metatype) *_mpt_iterator_range(MPT_STRUCT(value) *val)
{
	MPT_INTERFACE(metatype) *mt;
	MPT_INTERFACE(iterator) *it;
	struct _iter_ldata *d;
	MPT_STRUCT(range) r = { 0.0, 1.0 };
	double step = 0.1;
	int iv = 10;
	
	if (val) {
		const char *str;
		int ret;
		if (val->fmt) {
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
	if (!(mt = malloc(sizeof(*mt) + sizeof(*it) + sizeof(*d)))) {
		return 0;
	}
	mt->_vptr = &_vptr_linear_meta;
	
	it = (void *) (mt + 1);
	it->_vptr = &_vptr_linear_iter;
	
	d = (void *) (it + 1);
	d->base = r.min;
	d->step = step;
	d->elem = iv + 1;
	d->pos = 0;
	
	return mt;
}

