/*!
 * create iterator with constant difference between elements.
 */

#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "convert.h"
#include "meta.h"

#include "values.h"

struct _iter_ldata
{
	double base,  /* current/base value */
	       step;  /* advance step size */
	int pos, max; /* curr = first + step * pos */
};

static int iterAssign(struct _iter_ldata *it, MPT_STRUCT(value) val)
{
	double base = 0.0, step = 0.1;
	int32_t max = 10;
	int len, curr;
	
	if (!val.fmt) {
		const char *str;
		if (!(str = val.ptr)) {
			curr = 0;
		}
		else if ((curr = mpt_cdouble(&step, str, 0)) < 0) {
			return curr;
		}
		len = 0;
		if (curr) {
			double tmp;
			len = 1;
			if ((curr = mpt_cdouble(&tmp, str += curr, 0)) < 0) {
				return curr;
			}
			if (curr) {
				++len;
				base = step;
				step = tmp;
			}
		}
		if (curr) {
			const int32_t range[2] = { 1, INT_MAX };
			if ((curr = mpt_cint32(&max, str += curr, 0, range)) < 0) {
				return curr;
			}
			if (curr) ++len;
		}
	}
	else if (*val.fmt) {
		if ((curr = mpt_data_convert(&val.ptr, *val.fmt++, &step, 'd')) < 0) {
			return curr;
		}
		len = curr ? 1 : 0;
		if (curr && *val.fmt) {
			double tmp;
			if ((curr = mpt_data_convert(&val.ptr, *val.fmt++, &tmp, 'd')) < 0) {
				return curr;
			}
			if (curr) {
				++len;
				base = step;
				step = tmp;
			}
		}
		if (curr && *val.fmt) {
			if ((curr = mpt_data_convert(&val.ptr, *val.fmt++, &max, 'i')) < 0) {
				return curr;
			}
			if (curr) {
				++len;
				if (max < 0) {
					return MPT_ERROR(BadValue);
				}
				if (!max) {
					max = INT_MAX;
				}
			}
		}
	}
	else {
		len = 0;
	}
	it->base = base;
	it->step = step;
	it->pos = 0;
	it->max = max;
	
	return len;
}
static void iterUnref(MPT_INTERFACE(unrefable) *mt)
{
	free(mt);
}
static int iterConv(const MPT_INTERFACE(metatype) *mt, int t, void *ptr)
{
	struct _iter_ldata *it = (void *) (mt + 1);
	if (!t) {
		static const char fmt[] = { MPT_ENUM(TypeIterator), 'd', 0 };
		if (ptr) *((const char **) ptr) = fmt;
		return MPT_ENUM(TypeIterator);
	}
	if (t == MPT_ENUM(TypeIterator)
	    || t == MPT_ENUM(TypeIterator)) {
		if (ptr) *((void **) ptr) = it;
		return MPT_ENUM(TypeIterator);
	}
	if (t != 'd') {
		return MPT_ERROR(BadType);
	}
	if (it->pos >= it->max) {
		return MPT_ERROR(MissingData);
	}
	if (ptr) {
		*((double *) ptr) = it->base + it->pos * it->step;
	}
	return 'd';
}
static const MPT_INTERFACE_VPTR(iterator) iteratorLinear;
static MPT_INTERFACE(metatype) *iterClone(const MPT_INTERFACE(metatype) *mt)
{
	struct _iter_ldata *it = (void *) (mt + 1);
	MPT_INTERFACE(metatype) *copy;
	
	if (!(copy = malloc(sizeof(*copy) + sizeof(*it)))) {
		return 0;
	}
	copy->_vptr = &iteratorLinear.meta;
	memcpy(copy + 1, it, sizeof(*it));
	return copy;
}
static int iterAdvance(MPT_INTERFACE(iterator) *ptr)
{
	struct _iter_ldata *it = (void *) (ptr + 1);
	if (it->pos > it->max) {
		return MPT_ERROR(MissingData);
	}
	if (++it->pos == it->max) {
		return 0;
	}
	return 'd';
}
static int iterReset(MPT_INTERFACE(iterator) *ptr)
{
	struct _iter_ldata *it = (void *) (ptr + 1);
	it->pos = 0;
	return it->max - 1;
}
static const MPT_INTERFACE_VPTR(iterator) iteratorLinear = {
	{ { iterUnref }, iterConv, iterClone },
	iterAdvance,
	iterReset
};

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
extern MPT_INTERFACE(iterator) *_mpt_iterator_linear(MPT_STRUCT(value) val)
{
	MPT_INTERFACE(iterator) *iter;
	struct _iter_ldata it;
	
	if (iterAssign(&it, val) < 0) {
		return 0;
	}
	++it.max;
	if (!(iter = malloc(sizeof(*iter) + sizeof(it)))) {
		return 0;
	}
	iter->_vptr = &iteratorLinear;
	memcpy(iter + 1, &it, sizeof(it));
	
	return iter;
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
extern MPT_INTERFACE(iterator) *_mpt_iterator_range(MPT_STRUCT(value) val)
{
	MPT_INTERFACE(iterator) *iter;
	struct _iter_ldata it;
	
	if (iterAssign(&it, val) < 0) {
		return 0;
	}
	if (it.max) {
		it.step = (it.step - it.base) / it.max++;
	}
	if (!(iter = malloc(sizeof(*iter) + sizeof(it)))) {
		return 0;
	}
	iter->_vptr = &iteratorLinear;
	memcpy(iter + 1, &it, sizeof(it));
	
	return iter;
}

