/*!
 * create iterator with constant faktor between elements.
 */

#include <errno.h>
#include <stdlib.h>
#include <limits.h>
#include <float.h>
#include <string.h>

#include "types.h"
#include "convert.h"
#include "meta.h"

#include "values.h"

struct iterFactorData
{
	double   base,  /* iteration base value */
	         fact,  /* multiplication factor */
	         init;  /* start value */
	uint32_t elem,  /* total elements */
	         pos;   /* iteration position */
	double   curr;  /* current value */
};

MPT_STRUCT(iteratorFactor)
{
	MPT_INTERFACE(metatype) _mt;
	MPT_INTERFACE(iterator) _it;
	MPT_STRUCT(value) val;
	
	struct iterFactorData data;
};

/* convertable interface */
static int iterFactorConv(MPT_INTERFACE(convertable) *val, int type, void *ptr)
{
	MPT_STRUCT(iteratorFactor) *d = MPT_baseaddr(iteratorFactor, val, _mt);
	if (!type) {
		static const uint8_t fmt[] = { MPT_ENUM(TypeIteratorPtr), 0 };
		if (ptr) {
			*((const uint8_t **) ptr) = fmt;
			return 'd';
		}
		return MPT_ENUM(TypeIteratorPtr);
	}
	if (type == MPT_ENUM(TypeIteratorPtr)) {
		if (ptr) *((const void **) ptr) = (void *) (&d->_it);
		return 'd';
	}
	return MPT_ERROR(BadType);
}
/* metatype interface */
static void iterFactorUnref(MPT_INTERFACE(metatype) *mt)
{
	free(mt);
}
static uintptr_t iterFactorRef(MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
static MPT_INTERFACE(metatype) *iterFactorClone(const MPT_INTERFACE(metatype) *mt)
{
	MPT_INTERFACE(metatype) *ptr;
	
	if ((ptr = _mpt_iterator_factor(0))) {
		MPT_STRUCT(iteratorFactor) *from, *to;
		from = MPT_baseaddr(iteratorFactor, mt, _mt);
		to   = MPT_baseaddr(iteratorFactor, ptr, _mt);
		
		to->data = from->data;
	}
	return ptr;
}
/* iterator interface */
static const MPT_STRUCT(value) *iterFactorValue(MPT_INTERFACE(iterator) *it)
{
	MPT_STRUCT(iteratorFactor) *d = MPT_baseaddr(iteratorFactor, it, _it);
	if (d->data.pos >= d->data.elem) {
		return 0;
	}
	MPT_value_set(&d->val, 'd', &d->data.curr);
	return &d->val;
}
static int iterFactorAdvance(MPT_INTERFACE(iterator) *it)
{
	MPT_STRUCT(iteratorFactor) *d = MPT_baseaddr(iteratorFactor, it, _it);
	
	if (d->data.pos >= d->data.elem) {
		return MPT_ERROR(MissingData);
	}
	if (!d->data.pos++) {
		d->data.curr = d->data.base;
	} else {
		d->data.curr *= d->data.fact;
	}
	if (d->data.pos == d->data.elem) {
		return 0;
	}
	return 'd';
}
static int iterFactorReset(MPT_INTERFACE(iterator) *it)
{
	MPT_STRUCT(iteratorFactor) *d = MPT_baseaddr(iteratorFactor, it, _it);
	d->data.pos = 0;
	d->data.curr = d->data.init;
	return d->data.elem;
}

/*!
 * \ingroup mptValues
 * \brief create factor iterator
 * 
 * Create iterator advancing by factor.
 * Default factor is replaced by base to simulate
 * linear exponential growth.
 * Default initial iterator value is zero and
 * must be set to 1 (=base^0) manually if so desired.
 * 
 * \param conf  factor iterator parameters
 * 
 * \return factor iterator metatype
 */
extern MPT_INTERFACE(metatype) *_mpt_iterator_factor(MPT_STRUCT(value) *val)
{

	static const MPT_INTERFACE_VPTR(metatype) factorMeta = {
		{ iterFactorConv },
		iterFactorUnref,
		iterFactorRef,
		iterFactorClone
	};
	static const MPT_INTERFACE_VPTR(iterator) factorIter = {
		iterFactorValue,
		iterFactorAdvance,
		iterFactorReset
	};
	MPT_STRUCT(iteratorFactor) *data;
	struct iterFactorData fd = { 10.0, 10.0, 0.0, 10, 0, 0.0 };
	
	if (val) {
		uint32_t iter;
		
		if (val->_type == MPT_ENUM(TypeIteratorPtr)) {
			MPT_INTERFACE(iterator) *it = *((MPT_INTERFACE(iterator) * const *) val->_addr);
			int cont = 0, ret;
			
			if ((ret = mpt_iterator_consume(it, 'u', &iter)) < 0) {
				errno = EINVAL;
				return 0;
			}
			if (ret > 0) {
				++cont;
				ret = mpt_iterator_consume(it, 'd', &fd.base);
			}
			if (ret > 0) {
				++cont;
				ret = mpt_iterator_consume(it, 'd', &fd.fact);
			}
			if (ret > 0) {
				++cont;
				ret = mpt_iterator_consume(it, 'd', &fd.init);
			}
			if (ret >= 0) {
				++cont;
			}
			if (cont < 2) {
				if (fd.base < DBL_MIN) {
					errno = EINVAL;
					return 0;
				}
				fd.fact = fd.base;
			}
			fd.elem = iter + 1;
		}
		else if (val->_type == 's') {
			const char *str = *((const char **) val->_addr);
			int c;
			if ((c = mpt_string_nextvis(&str)) != '(') {
				errno = EINVAL;
				return 0;
			}
			if ((c = mpt_cuint32(&iter, str + 1, 0, 0)) < 0) {
				errno = EINVAL;
				return 0;
			}
			fd.elem = iter + 1;
			str += c + 1;
			/* base value for multiplication */
			if ((c = mpt_string_nextvis(&str)) == ':') {
				if ((c = mpt_cdouble(&fd.base, str + 1, 0)) < 0) {
					errno = EINVAL;
					return 0;
				}
				str += c + 1;
			}
			/* set factor, default to base */
			if ((c = mpt_string_nextvis(&str)) == ':') {
				if (str[1] == ':') {
					if (fd.base < DBL_MIN) {
						errno = EINVAL;
						return 0;
					} else {
						fd.fact = fd.base;
					}
					c = 0;
				}
				else if ((c = mpt_cdouble(&fd.fact, str + 1, 0)) < 0
				      || fd.fact < DBL_MIN) {
					errno = EINVAL;
					return 0;
				}
				str += c + 1;
				/* initial iterator value */
				if ((c = mpt_string_nextvis(&str)) == ':') {
					if ((c = mpt_cdouble(&fd.init, str + 1, 0)) < 0) {
						errno = EINVAL;
						return 0;
					}
					str += c + 1;
				}
			}
			else if (fd.base < DBL_MIN) {
				errno = EINVAL;
				return 0;
			} else {
				fd.fact = fd.base;
			}
			if ((c = mpt_string_nextvis(&str)) != ')') {
				errno = EINVAL;
				return 0;
			}
		}
		else {
			errno = EINVAL;
			return 0;
		}
	}
	if (!(data = malloc(sizeof(*data)))) {
		return 0;
	}
	data->_mt._vptr = &factorMeta;
	data->_it._vptr = &factorIter;
	
	MPT_value_set(&data->val, 0, 0);
	
	data->data = fd;
	data->data.curr = data->data.init;
	
	return &data->_mt;
}

