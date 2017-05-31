/*!
 * create iterator with constant faktor between elements.
 */

#include <stdlib.h>
#include <limits.h>
#include <string.h>

#include "convert.h"
#include "meta.h"

#include "values.h"

struct _iter_fdata
{
	double first, base, /* first/base value */
	       curr,        /* current value */
	       fact;        /* multiplication factor */
	int    pos, max;    /* iterations left */
};

static void iterUnref(MPT_INTERFACE(unrefable) *mt)
{
	free(mt);
}
static int iterAssign(struct _iter_fdata *it, MPT_STRUCT(value) val)
{
	double first = 0.0, base = 1.0, fact = 10.;
	int32_t max = 10;
	int len, curr;
	
	if (!val.fmt) {
		const char *str;
		if (!(str = val.ptr)) {
			curr = 0;
		}
		else if ((curr = mpt_cdouble(&fact, str, 0)) < 0) {
			return curr;
		}
		len = 0;
		if (curr) {
			const int32_t range[2] = { 1, INT_MAX };
			len = 1;
			if ((curr = mpt_cint32(&max, str += curr, 0, range)) < 0) {
				return curr;
			}
			if (curr) ++len;
		}
		if (curr) {
			if ((curr = mpt_cdouble(&base, str += curr, 0)) < 0) {
				return curr;
			}
			if (curr) ++len;
		}
		if (curr) {
			if ((curr = mpt_cdouble(&first, str += curr, 0)) < 0) {
				return curr;
			}
			if (curr) ++len;
		}
	}
	else if (!*val.fmt) {
		len = 0;
	}
	else {
		if ((curr = mpt_data_convert(&val.ptr, *val.fmt++, &fact, 'd')) < 0) {
			return curr;
		}
		len = curr ? 1 : 0;
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
		if (curr && *val.fmt) {
			if ((curr = mpt_data_convert(&val.ptr, *val.fmt++, &base, 'd')) < 0) {
				return curr;
			}
			if (curr) ++len;
		}
		if (curr && *val.fmt) {
			if ((curr = mpt_data_convert(&val.ptr, *val.fmt++, &first, 'd')) < 0) {
				return curr;
			}
			if (curr) ++len;
		}
	}
	it->first = first;
	it->base = base;
	it->curr = first;
	it->fact = fact;
	it->pos = 0;
	it->max = max;
	
	return len;
}
static int iterConv(const MPT_INTERFACE(metatype) *mt, int t, void *ptr)
{
	struct _iter_fdata *it = (void *) (mt + 1);
	if (!t) {
		static const char fmt[] = { MPT_ENUM(TypeIterator), 'd', 0 };
		if (ptr) *((const char **) ptr) = fmt;
		return 0;
	}
	if (t == MPT_ENUM(TypeIterator)
	    || t == MPT_ENUM(TypeIterator)) {
		if (ptr) *((void **) ptr) = it;
		return MPT_ENUM(TypeIterator);
	}
	if (t != 'd') {
		return MPT_ERROR(BadType);
	}
	if (it->pos > it->max) {
		return MPT_ERROR(MissingData);
	}
	if (ptr) {
		*((double *) ptr) = it->curr;
	}
	return 'd';
}
static MPT_INTERFACE_VPTR(iterator) iteratorFactor;
static MPT_INTERFACE(metatype) *iterClone(const MPT_INTERFACE(metatype) *mt)
{
	MPT_INTERFACE(metatype) *copy;
	struct _iter_fdata *it = (void *) (mt + 1);
	
	if (!(copy = malloc(sizeof(*copy) + sizeof(*it)))) {
		return 0;
	}
	copy->_vptr = &iteratorFactor.meta;
	memcpy(copy + 1, it, sizeof(*it));
	return copy;
}
static int iterAdvance(MPT_INTERFACE(iterator) *ptr)
{
	struct _iter_fdata *it = (void *) (ptr + 1);
	
	if (it->pos > it->max) {
		return MPT_ERROR(MissingData);
	}
	if (it->pos == it->max) {
		return 0;
	}
	if (!it->pos++) {
		it->curr = it->base;
	} else {
		it->curr *= it->fact;
	}
	return 'd';
}
static int iterReset(MPT_INTERFACE(iterator) *ptr)
{
	struct _iter_fdata *it = (void *) (ptr + 1);
	it->pos = 0;
	it->curr = it->first;
	return it->max;
}

static MPT_INTERFACE_VPTR(iterator) iteratorFactor = {
	{ { iterUnref }, iterConv, iterClone },
	iterAdvance,
	iterReset
};

/*!
 * \ingroup mptValues
 * \brief create factor iterator
 * 
 * Create iterator advancing by factor.
 * 
 * \param conf  factor iterator parameters
 * 
 * \return factor iterator metatype
 */
extern MPT_INTERFACE(iterator) *_mpt_iterator_factor(MPT_STRUCT(value) val)
{
	MPT_INTERFACE(iterator) *iter;
	struct _iter_fdata it;
	
	if (iterAssign(&it, val) < 0) {
		return 0;
	}
	++it.max;
	if (!(iter = malloc(sizeof(*iter) + sizeof(it)))) {
		return 0;
	}
	iter->_vptr = &iteratorFactor;
	memcpy(iter + 1, &it, sizeof(it));
	
	return iter;
}

