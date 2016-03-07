/*!
 * create iterator with constant faktor between elements.
 */

#include <stdlib.h>
#include <limits.h>
#include <string.h>

#include "convert.h"

#include "values.h"

struct _iter_fdata
{
	MPT_INTERFACE(metatype) mt;
	double curr, base, /* current/base value */
	       fact;       /* multiplication factor */
	int    pos, max;   /* iterations left */
};

static void iterUnref(MPT_INTERFACE(metatype) *mt)
{
	free(mt);
}
static int iterAssign(MPT_INTERFACE(metatype) *mt, const MPT_STRUCT(value) *val)
{
	struct _iter_fdata *it = (void *) mt;
	const char *str;
	double first = 0.0, base = 1.0, fact = 10.;
	int32_t max = 10;
	int len, curr;
	
	if (!val) {
		len = 0;
	}
	else if (!(str = val->fmt)) {
		if (!(str = val->ptr)) {
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
	else if (*str) {
		const void *ptr = val->ptr;
		
		if ((curr = mpt_data_convert(&ptr, *str++, &fact, 'd' | MPT_ENUM(ValueConsume))) < 0) {
			return curr;
		}
		len = curr ? 1 : 0;
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
		if (curr && *str) {
			if ((curr = mpt_data_convert(&ptr, *str++, &base, 'd' | MPT_ENUM(ValueConsume))) < 0) {
				return curr;
			}
			if (curr) ++len;
		}
		if (curr && *str) {
			if ((curr = mpt_data_convert(&ptr, *str++, &first, 'd' | MPT_ENUM(ValueConsume))) < 0) {
				return curr;
			}
			if (curr) ++len;
		}
	}
	else {
		len = 0;
	}
	it->curr = first;
	it->base = base;
	it->fact = fact;
	it->pos = 0;
	it->max = max;
	
	return len;
}
static int iterConv(MPT_INTERFACE(metatype) *mt, int t, void *ptr)
{
	struct _iter_fdata *it = (void *) mt;
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
		*((double *) ptr) = it->curr;
	}
	if (t & MPT_ENUM(ValueConsume)) {
		if (!it->pos++) {
			it->curr = it->base;
		} else {
			it->curr *= it->fact;
		}
		return 'd' | MPT_ENUM(ValueConsume);
	}
	return 'd';
}
static MPT_INTERFACE(metatype) *iterClone(MPT_INTERFACE(metatype) *mt)
{
	struct _iter_fdata *c;
	
	if (!(c = malloc(sizeof*c))) {
		return 0;
	}
	return memcpy(c, mt, sizeof(*c));
}

static MPT_INTERFACE_VPTR(metatype) iteratorFactor = {
	iterUnref,
	iterAssign,
	iterConv,
	iterClone
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
extern MPT_INTERFACE(metatype) *_mpt_iterator_factor(const char *conf)
{
	struct _iter_fdata it;
	MPT_STRUCT(value) val;
	
	if (!(val.ptr = conf)) {
		return 0;
	}
	val.fmt = 0;
	
	if (iterAssign(&it.mt, &val) < 0) {
		return 0;
	}
	it.mt._vptr = &iteratorFactor;
	
	return iterClone(&it.mt);
}

