/*!
 * create iterator with constant faktor between elements.
 */

#include <errno.h>
#include <stdlib.h>
#include <limits.h>
#include <float.h>
#include <string.h>

#include "parse.h"
#include "convert.h"
#include "meta.h"

#include "values.h"

struct _iter_fdata
{
	double   base,  /* iteration base value */
	         fact,  /* multiplication factor */
	         init;  /* start value */
	uint32_t elem,  /* total elements */
	         pos;   /* iteration position */
	double   curr;  /* current value */
};

static void iterUnref(MPT_INTERFACE(reference) *mt)
{
	free(mt);
}
static uintptr_t iterRef(MPT_INTERFACE(reference) *ref)
{
	(void) ref;
	return 0;
}
static int iterGet(MPT_INTERFACE(iterator) *mt, int type, void *ptr)
{
	struct _iter_fdata *it = (void *) (mt + 1);
	if (!type) {
		static const char fmt[] = { 'd', 'd', 'd', 'u' };
		MPT_STRUCT(value) *val;
		if ((val = ptr)) {
			val->fmt = fmt;
			val->ptr = it;
		}
		return 0;
	}
	if (it->pos >= it->elem) {
		return 0;
	}
	if (type == 'd') {
		if (ptr) {
			*((double *) ptr) = it->curr;
		}
		return 'd';
	}
	if (type == 'f') {
		if (ptr) {
			*((float *) ptr) = it->curr;
		}
		return 'd';
	}
	return MPT_ERROR(BadType);
}
static int iterAdvance(MPT_INTERFACE(iterator) *ptr)
{
	struct _iter_fdata *it = (void *) (ptr + 1);
	
	if (it->pos >= it->elem) {
		return MPT_ERROR(MissingData);
	}
	if (!it->pos++) {
		it->curr = it->base;
	} else {
		it->curr *= it->fact;
	}
	if (it->pos == it->elem) {
		return 0;
	}
	return 'd';
}
static int iterReset(MPT_INTERFACE(iterator) *ptr)
{
	struct _iter_fdata *it = (void *) (ptr + 1);
	it->pos = 0;
	it->curr = it->init;
	return it->elem;
}

static MPT_INTERFACE_VPTR(iterator) iteratorFactor = {
	{ iterUnref, iterRef },
	iterGet,
	iterAdvance,
	iterReset
};

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
extern MPT_INTERFACE(iterator) *_mpt_iterator_factor(MPT_STRUCT(value) *val)
{
	MPT_INTERFACE(iterator) *iter;
	struct _iter_fdata it = { 10.0, 10.0, 0.0, 10, 0, 0.0 };
	int len;
	
	if (val) {
		const char *str;
		uint32_t iter;
		if ((str = val->fmt)) {
			int cont;
			if ((len = mpt_value_read(val, "u", &iter)) <= 0) {
				errno = EINVAL;
				return 0;
			}
			if ((cont = mpt_value_read(val, "ddd", &it)) < 1) {
				errno = EINVAL;
				return 0;
			}
			if (cont < 2) {
				if (it.base < DBL_MIN) {
					errno = EINVAL;
					return 0;
				}
				it.fact = it.base;
			}
			len += cont;
			it.elem = iter + 1;
		}
		else if ((str = val->ptr)) {
			int c;
			if ((c = mpt_string_nextvis(&str)) != '(') {
				errno = EINVAL;
				return 0;
			}
			if ((c = mpt_cuint32(&iter, str + 1, 0, 0)) < 0) {
				errno = EINVAL;
				return 0;
			}
			it.elem = iter + 1;
			str += c + 1;
			/* base value for multiplication */
			if ((c = mpt_string_nextvis(&str)) == ':') {
				if ((c = mpt_cdouble(&it.base, str + 1, 0)) < 0) {
					errno = EINVAL;
					return 0;
				}
				str += c + 1;
			}
			/* set factor, default to base */
			if ((c = mpt_string_nextvis(&str)) == ':') {
				if (str[1] == ':') {
					if (it.base < DBL_MIN) {
						errno = EINVAL;
						return 0;
					} else {
						it.fact = it.base;
					}
					c = 0;
				}
				else if ((c = mpt_cdouble(&it.fact, str + 1, 0)) < 0
				      || it.fact < DBL_MIN) {
					errno = EINVAL;
					return 0;
				}
				str += c + 1;
				/* initial iterator value */
				if ((c = mpt_string_nextvis(&str)) == ':') {
					if ((c = mpt_cdouble(&it.init, str + 1, 0)) < 0) {
						errno = EINVAL;
						return 0;
					}
					str += c + 1;
				}
			}
			else if (it.base < DBL_MIN) {
				errno = EINVAL;
				return 0;
			} else {
				it.fact = it.base;
			}
			if ((c = mpt_string_nextvis(&str)) != ')') {
				errno = EINVAL;
				return 0;
			}
		}
	}
	if (!(iter = malloc(sizeof(*iter) + sizeof(it)))) {
		return 0;
	}
	iter->_vptr = &iteratorFactor;
	it.curr = it.init;
	memcpy(iter + 1, &it, sizeof(it));
	
	return iter;
}

