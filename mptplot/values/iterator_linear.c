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

MPT_STRUCT(iteratorLinear)
{
	MPT_INTERFACE(metatype) _mt;
	MPT_INTERFACE(iterator) _it;
	MPT_STRUCT(value) val;
	
	double   base,  /* current/base value */
	         step,  /* advance step size */
	         curr;  /* current value */
	uint32_t elem,  /* number of elements */
	         pos;
};

/* convertable interface */
static int iterConv(MPT_INTERFACE(convertable) *val, int type, void *ptr)
{
	MPT_STRUCT(iteratorLinear) *d = MPT_baseaddr(iteratorLinear, val, _mt);
	
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
	MPT_INTERFACE(metatype) *ptr;
	
	if ((ptr = _mpt_iterator_range(0))) {
		MPT_STRUCT(iteratorLinear) *dest, *from;
		from = MPT_baseaddr(iteratorLinear, mt, _mt);
		dest = MPT_baseaddr(iteratorLinear, ptr, _mt);
		
		dest->base = from->base;
		dest->step = from->step;
		dest->elem = from->elem;
		dest->pos  = from->pos;
	}
	return ptr;
}
/* iterator interface */
static const MPT_STRUCT(value) *iterValue(MPT_INTERFACE(iterator) *it)
{
	MPT_STRUCT(iteratorLinear) *d = MPT_baseaddr(iteratorLinear, it, _it);
	if (d->pos >= d->elem) {
		return 0;
	}
	d->curr = d->base + d->pos * d->step;
	MPT_value_set(&d->val, 'd', &d->curr);
	
	return &d->val;
}
static int iterAdvance(MPT_INTERFACE(iterator) *it)
{
	MPT_STRUCT(iteratorLinear) *d = MPT_baseaddr(iteratorLinear, it, _it);
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
	MPT_STRUCT(iteratorLinear) *d = MPT_baseaddr(iteratorLinear, it, _it);
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
	iterValue,
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
	MPT_STRUCT(iteratorLinear) *data;
	if (len < 2) {
		errno = EINVAL;
		return 0;
	}
	if (!(data = malloc(sizeof(*data)))) {
		return 0;
	}
	data->_mt._vptr = &_vptr_linear_meta;
	data->_it._vptr = &_vptr_linear_iter;
	
	MPT_value_set(&data->val, 0, 0);
	
	data->base = start;
	data->step = (end - start) / (len - 1);
	data->elem = len;
	data->pos  = 0;
	
	return &data->_mt;
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
	else if (val->type == MPT_ENUM(TypeIteratorPtr)) {
		MPT_INTERFACE(iterator) *it = *((void * const *) val->ptr);
		const MPT_STRUCT(value) *val = it->_vptr->value(it);
		MPT_TYPE(data_converter) conv;
		
		if (!(conv = mpt_data_converter(val->type))
		 || (ret = conv(val->ptr, 'u', &iv)) < 0
		 || (ret = mpt_range_set(&r, val)) < 0) {
			errno = EINVAL;
			return 0;
		}
	}
	else if (val->type == 's') {
		const char *str = *((char * const *) val->ptr);;
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
	MPT_STRUCT(iteratorLinear) *data;
	MPT_STRUCT(range) r = { 0.0, 1.0 };
	double step = 0.1;
	int iv = 10;
	
	if (val) {
		const char *str;
		int ret;
		if (val->type == MPT_ENUM(TypeIteratorPtr)) {
			MPT_INTERFACE(iterator) *it = *((void * const *) val->ptr);
			
			if ((ret = mpt_range_set(&r, val)) < 0) {
				errno = EINVAL;
				return 0;
			}
			step = (r.max - r.min) / 10;
			if (ret >= 2 && ((ret = mpt_iterator_consume(it, 'd', &step)) < 0)) {
				errno = EINVAL;
				return 0;
			}
		}
		else if ((val->type == 's') && (str = *((const char **) val->ptr))) {
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
	if (!(data = malloc(sizeof(*data)))) {
		return 0;
	}
	data->_mt._vptr = &_vptr_linear_meta;
	data->_it._vptr = &_vptr_linear_iter;
	
	MPT_value_set(&data->val, 0, 0);
	
	data->base = r.min;
	data->step = step;
	data->elem = iv + 1;
	data->pos = 0;
	
	return &data->_mt;
}

