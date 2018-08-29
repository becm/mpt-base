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
/* reference interface */
static void iterFactorUnref(MPT_INTERFACE(instance) *in)
{
	free(in);
}
static uintptr_t iterFactorRef(MPT_INTERFACE(instance) *in)
{
	(void) in;
	return 0;
}
/* metatype interface */
static int iterFactorConv(const MPT_INTERFACE(metatype) *mt, int type, void *ptr)
{
	if (!type) {
		static const uint8_t fmt[] = { MPT_ENUM(TypeIterator), 0 };
		if (ptr) {
			*((const uint8_t **) ptr) = fmt;
			return 'd';
		}
		return MPT_ENUM(TypeIterator);
	}
	if (type == MPT_type_pointer(MPT_ENUM(TypeIterator))) {
		if (ptr) *((const void **) ptr) = mt + 1;
		return 'd';
	}
	return MPT_ERROR(BadType);
}
static MPT_INTERFACE(metatype) *iterFactorClone(const MPT_INTERFACE(metatype) *mt)
{
	MPT_INTERFACE(metatype) *ptr;
	
	if ((ptr = _mpt_iterator_factor(0))) {
		struct _iter_fdata *val, *d;
		val = (void *) (ptr + 2);
		d   = (void *) (mt  + 2);
		*val = *d;
	}
	return ptr;
}
/* iterator interface */
static int iterFactorGet(MPT_INTERFACE(iterator) *it, int type, void *ptr)
{
	struct _iter_fdata *d = (void *) (it + 1);
	if (!type) {
		static const uint8_t fmt[] = "df";
		if (ptr) {
			*((const uint8_t **) ptr) = fmt;
		}
		return d->pos;
	}
	if (d->pos >= d->elem) {
		return 0;
	}
	if (type == 'd') {
		if (ptr) *((double *) ptr) = d->curr;
		return 'd';
	}
	if (type == 'f') {
		if (ptr) *((float *) ptr) = d->curr;
		return 'd';
	}
	return MPT_ERROR(BadType);
}
static int iterFactorAdvance(MPT_INTERFACE(iterator) *it)
{
	struct _iter_fdata *d = (void *) (it + 1);
	
	if (d->pos >= d->elem) {
		return MPT_ERROR(MissingData);
	}
	if (!d->pos++) {
		d->curr = d->base;
	} else {
		d->curr *= d->fact;
	}
	if (d->pos == d->elem) {
		return 0;
	}
	return 'd';
}
static int iterFactorReset(MPT_INTERFACE(iterator) *it)
{
	struct _iter_fdata *d = (void *) (it + 1);
	d->pos = 0;
	d->curr = d->init;
	return d->elem;
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
		{ iterFactorUnref, iterFactorRef },
		iterFactorConv,
		iterFactorClone
	};
	static const MPT_INTERFACE_VPTR(iterator) factorIter = {
		iterFactorGet,
		iterFactorAdvance,
		iterFactorReset
	};
	MPT_INTERFACE(metatype) *mt;
	MPT_INTERFACE(iterator) *it;
	struct _iter_fdata fd = { 10.0, 10.0, 0.0, 10, 0, 0.0 };
	int len;
	
	if (val) {
		const char *str;
		uint32_t iter;
		if (val->fmt) {
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
				if (fd.base < DBL_MIN) {
					errno = EINVAL;
					return 0;
				}
				fd.fact = fd.base;
			}
			len += cont;
			fd.elem = iter + 1;
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
	}
	if (!(mt = malloc(sizeof(*mt) + sizeof(*it) + sizeof(fd)))) {
		return 0;
	}
	mt->_vptr = &factorMeta;
	
	it = (void *) (mt + 1);
	it->_vptr = &factorIter;
	
	fd.curr = fd.init;
	memcpy(it + 1, &fd, sizeof(fd));
	
	return mt;
}

