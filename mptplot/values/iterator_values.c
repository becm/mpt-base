/*!
 * create iterator with double values read from string.
 */

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>

#include "convert.h"
#include "meta.h"

#include "values.h"

struct _iter_sdata
{
	const char *next;  /* next position in string */
	double curr;      /* current iterator value */
};
struct _iter_fdata
{
	int curr, max;     /* remaining/total values */
};

static void iterUnref(MPT_INTERFACE(unrefable) *mt)
{
	free(mt);
}
static int iterStrConv(const MPT_INTERFACE(metatype) *mt, int t, void *ptr)
{
	struct _iter_sdata *it = (void *) (mt + 1);
	if (!t) {
		static const char fmt[] = { 'd', 0 };
		if (ptr) *((const char **) ptr) = fmt;
		return 0;
	}
	if (t != 'd') {
		return MPT_ERROR(BadType);
	}
	if (!it->next) {
		return 0;
	}
	if (isnan(it->curr)) {
		return MPT_ERROR(BadValue);
	}
	if (ptr) {
		*((double *) ptr) = it->curr;
	}
	return 's';
}
static const MPT_INTERFACE_VPTR(iterator) iteratorStr;
static MPT_INTERFACE(metatype) *iterStrClone(const MPT_INTERFACE(metatype) *mt)
{
	MPT_INTERFACE(iterator) *iter;
	struct _iter_sdata *it = (void *) (mt + 1);
	const char *val;
	size_t len, pos;
	double curr;
	
	curr = it->curr;
	val = (const char *) (it + 1);
	pos = it->next - val;
	len = strlen(val);
	
	if (!(iter = malloc(sizeof(*iter) + sizeof(*it) + len + 1))) {
		return 0;
	}
	it = (void *) (iter + 1);
	val = memcpy(it + 1, val, len + 1);
	
	iter->_vptr = &iteratorStr;
	it->next = val + pos;
	it->curr = curr;
	
	return memcpy(it, mt, sizeof(*it));
}
static int iterStrAdvance(MPT_INTERFACE(iterator) *ptr)
{
	struct _iter_sdata *it = (void *) (ptr + 1);
	int len;
	if (!it->next) {
		return MPT_ERROR(MissingData);
	}
	if (!*it->next || !(len = mpt_cdouble(&it->curr, it->next, 0))) {
		it->next = 0;
		return 0;
	}
	if (len < 0 || isnan(it->curr)) {
		return MPT_ERROR(BadValue);
	}
	it->next += len;
	return 'd';
}
static int iterStrReset(MPT_INTERFACE(iterator) *ptr)
{
	struct _iter_sdata *it = (void *) (ptr + 1);
	const char *base = (const char *) (it + 1);
	int len;
	if (!(len = mpt_cdouble(&it->curr, base, 0))) {
		it->curr = 0;
		return MPT_ERROR(MissingData);
	}
	if (len < 0 || isnan(it->curr)) {
		return MPT_ERROR(BadValue);
	}
	it->next = base + len;
	return 0;
}

static const MPT_INTERFACE_VPTR(iterator) iteratorStr = {
	{ { iterUnref }, iterStrConv, iterStrClone },
	iterStrAdvance,
	iterStrReset
};


static int iterFltConv(const MPT_INTERFACE(metatype) *mt, int t, void *ptr)
{
	struct _iter_fdata *it = (void *) (mt + 1);
	const double *base;
	if (!t) {
		static const char fmt[] = { 'd', 0 };
		if (ptr) *((const char **) ptr) = fmt;
		return 0;
	}
	if (t != 'd') {
		return MPT_ERROR(BadType);
	}
	if (it->curr >= it->max) {
		return MPT_ERROR(MissingData);
	}
	base = ((const double *) (it + 1)) + it->curr;
	if (isnan(*base)) {
		return MPT_ERROR(BadValue);
	}
	if (ptr) {
		*((double *) ptr) = *base;
	}
	return 'd';
}
static const MPT_INTERFACE_VPTR(iterator) iteratorFlt;
static MPT_INTERFACE(metatype) *iterFltClone(const MPT_INTERFACE(metatype) *mt)
{
	MPT_INTERFACE(metatype) *copy;
	struct _iter_fdata *it = (void *) (mt + 1);
	int len;
	
	len = sizeof(*it) + it->max * sizeof(double);
	if (!(copy = malloc(sizeof(*copy) + len))) {
		return 0;
	}
	copy->_vptr = &iteratorFlt.meta;
	memcpy(copy + 1, it, len + 1);
	
	return copy;
}
static int iterFltAdvance(MPT_INTERFACE(iterator) *ptr)
{
	struct _iter_fdata *it = (void *) (ptr + 1);
	if (it->curr > it->max) {
		return MPT_ERROR(MissingData);
	}
	if (++it->curr == it->max) {
		return 0;
	}
	return 'd';
}
static int iterFltReset(MPT_INTERFACE(iterator) *ptr)
{
	struct _iter_fdata *it = (void *) (ptr + 1);
	it->curr = 0;
	return it->max;
}

static const MPT_INTERFACE_VPTR(iterator) iteratorFlt = {
	{ { iterUnref }, iterFltConv, iterFltClone },
	iterFltAdvance,
	iterFltReset
};

/*!
 * \ingroup mptValues
 * \brief create value iterator
 * 
 * Create iterator with values taken from text.
 * 
 * \param conf	value iterator parameters
 * 
 * \return iterator interface
 */
extern MPT_INTERFACE(iterator) *_mpt_iterator_values(MPT_STRUCT(value) val)
{
	MPT_INTERFACE(iterator) *iter;
	struct _iter_sdata *sit;
	const char *str;
	size_t len;
	double flt;
	int adv;
	
	if ((str = val.fmt)) {
		struct _iter_sdata *fit;
		double *dest;
		size_t i;
		if (!(len = strlen(str))) {
			errno = EINVAL;
			return 0;
		}
		for (i = 0; i < len; ++i) {
			char curr = *str;
			if (curr == 'f') continue;
			if (curr == 'd') continue;
			if (curr == 's') continue;
			errno = EINVAL;
			return 0;
		}
		if (!(iter = malloc(sizeof(*iter) + sizeof(*fit) + len * sizeof(flt)))) {
			return 0;
		}
		iter->_vptr = &iteratorFlt;
		fit = (void *) (iter + 1);
		dest = (void *) (fit + 1);
		
		for (i = 0; i < len; ++i) {
			char curr = *val.fmt;
			if (curr == 'f') {
				const float *ptr = val.ptr;
				dest[i] = *ptr;
				val.ptr = ptr + 1;
				continue;
			}
			if (curr == 'd') {
				const double *ptr = val.ptr;
				dest[i] = *ptr;
				val.ptr = ptr + 1;
				continue;
			}
			if (curr == 's') {
				const char *curr;
				char * const *ptr = val.ptr;
				if (!(curr = *ptr)) {
					dest[i] = 0;
				}
				if ((adv = mpt_cdouble(dest + i, curr, 0)) < 0) {
					free(iter);
					errno = EINVAL;
					return 0;
				}
				if (!adv) {
					dest[i] = 0;
				}
				else if (isnan(dest[i])) {
					free(iter);
					errno = EINVAL;
					return 0;
				}
				val.ptr = ptr + 1;
				continue;
			}
			free(iter);
			errno = EINVAL;
			return 0;
		}
		
	}
	/* convert first value */
	str = val.ptr;
	if ((adv = mpt_cdouble(&flt, str, 0)) <= 0 || isnan(flt)) {
		errno = EINVAL;
		return 0;
	}
	len = strlen(str) + 1;
	
	if (!(iter = malloc(sizeof(*iter) + sizeof(*sit) + len))) {
		return 0;
	}
	sit = (void *) (iter + 1);
	
	iter->_vptr = &iteratorStr;
	
	str = memcpy(sit + 1, str, len);
	sit->next = str + adv;
	sit->curr = flt;
	
	return iter;
}
