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
	MPT_INTERFACE(metatype) mt;
	double base,  /* current/base value */
	       step;  /* advance step size */
	int pos, max; /* curr = first + step * pos */
};

static void iterUnref(MPT_INTERFACE(metatype) *mt)
{
	free(mt);
}
static int iterAssign(MPT_INTERFACE(metatype) *mt, const MPT_STRUCT(value) *val)
{
	struct _iter_ldata *it = (void *) mt;
	const char *str;
	double base = 0.0, step = 0.1;
	int32_t max = 10;
	int len, curr;
	
	if (!val) {
		len = 0;
	}
	else if (!(str = val->fmt)) {
		if (!(str = val->ptr)) {
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
	else if (*str) {
		const void *ptr = val->ptr;
		
		if ((curr = mpt_data_convert(&ptr, *str++, &step, 'd' | MPT_ENUM(ValueConsume))) < 0) {
			return curr;
		}
		len = curr ? 1 : 0;
		if (curr && *str) {
			double tmp;
			if ((curr = mpt_data_convert(&ptr, *str++, &tmp, 'd' | MPT_ENUM(ValueConsume))) < 0) {
				return curr;
			}
			if (curr) {
				++len;
				base = step;
				step = tmp;
			}
		}
		if (curr && *str) {
			if ((curr = mpt_data_convert(&ptr, *str++, &max, 'i' | MPT_ENUM(ValueConsume))) < 0) {
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
static int iterConv(MPT_INTERFACE(metatype) *mt, int t, void *ptr)
{
	struct _iter_ldata *it = (void *) mt;
	if (!t) {
		static const char fmt[] = { 'd', 0 };
		if (ptr) *((const char **) ptr) = fmt;
		return 0;
	}
	if ((t & 0xff) != 'd') {
		return MPT_ERROR(BadType);
	}
	if (it->pos > it->max) {
		return 0;
	}
	if (ptr) {
		*((double *) ptr) = it->base + it->pos * it->step;
	}
	if (t & MPT_ENUM(ValueConsume)) {
		++it->pos;
		return 'd' | MPT_ENUM(ValueConsume);
	}
	return 'd';
}
static MPT_INTERFACE(metatype) *iterClone(const MPT_INTERFACE(metatype) *mt)
{
	struct _iter_ldata *c;
	
	if (!(c = malloc(sizeof*c))) {
		return 0;
	}
	return memcpy(c, mt, sizeof(*c));
}
static const MPT_INTERFACE_VPTR(metatype) iteratorLinear = {
	iterUnref,
	iterAssign,
	iterConv,
	iterClone
};

/*!
 * \ingroup mptValues
 * \brief create linear iterator
 * 
 * Create iterator with linear advancing.
 * 
 * \param conf  linear iterator parameters
 * 
 * \return linear iterator metatype
 */
extern MPT_INTERFACE(metatype) *_mpt_iterator_linear(const char *conf)
{
	struct _iter_ldata it;
	MPT_STRUCT(value) val;
	
	if (!(val.ptr = conf)) {
		return 0;
	}
	val.fmt = 0;
	
	if (iterAssign(&it.mt, &val) < 0) {
		return 0;
	}
	it.mt._vptr = &iteratorLinear;
	
	return iterClone(&it.mt);
}

/*!
 * \ingroup mptValues
 * \brief create range iterator
 * 
 * Create iterator with linear advancing.
 * 
 * \param conf  range iterator parameters
 * 
 * \return linear iterator metatype
 */
extern MPT_INTERFACE(metatype) *_mpt_iterator_range(const char *conf)
{
	MPT_INTERFACE(metatype) *iter;
	struct _iter_ldata *it;
	
	if (!(iter = _mpt_iterator_linear(conf))) {
		return 0;
	}
	it = (void *) iter;
	
	if (it->max) {
		it->step = (it->step - it->base) / it->max;
	}
	return iter;
}

