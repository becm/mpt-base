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

static void iterUnref(MPT_INTERFACE(unrefable) *mt)
{
	free(mt);
}
static int iterValGet(MPT_INTERFACE(iterator) *it, int type, void *ptr)
{
	struct _iter_sdata *data = (void *) (it + 1);
	if (!type) {
		MPT_STRUCT(value) *val;
		if ((val = ptr)) {
			val->fmt = 0;
			val->ptr = (void *) (data + 1);
		}
		return data->next - (char *) (data + 1);
	}
	if (!data->next) {
		return 0;
	}
	if (isnan(data->curr)) {
		return MPT_ERROR(BadValue);
	}
	if (type == 'd') {
		if (ptr) {
			*((double *) ptr) = data->curr;
		}
		return 's';
	}
	if (type == 'f') {
		if (ptr) {
			*((float *) ptr) = data->curr;
		}
		return 's';
	}
	return MPT_ERROR(BadType);
}
static int iterValAdvance(MPT_INTERFACE(iterator) *it)
{
	struct _iter_sdata *data = (void *) (it + 1);
	int len;
	if (!data->next) {
		return MPT_ERROR(MissingData);
	}
	if (!*data->next || !(len = mpt_cdouble(&data->curr, data->next, 0))) {
		data->next = 0;
		return 0;
	}
	if (len < 0 || isnan(data->curr)) {
		return MPT_ERROR(BadValue);
	}
	data->next += len;
	return 'd';
}
static int iterValReset(MPT_INTERFACE(iterator) *it)
{
	struct _iter_sdata *data = (void *) (it + 1);
	const char *base = (const char *) (data + 1);
	int len;
	if (!(len = mpt_cdouble(&data->curr, base, 0))) {
		data->curr = 0;
		return MPT_ERROR(MissingData);
	}
	if (len < 0 || isnan(data->curr)) {
		return MPT_ERROR(BadValue);
	}
	data->next = base + len;
	return 0;
}

static const MPT_INTERFACE_VPTR(iterator) iteratorStr = {
	{ iterUnref },
	iterValGet,
	iterValAdvance,
	iterValReset
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
extern MPT_INTERFACE(iterator) *mpt_iterator_values(const char *val)
{
	MPT_INTERFACE(iterator) *iter;
	struct _iter_sdata *data;
	size_t len;
	double flt;
	int adv;
	
	if (!val) {
		errno = EINVAL;
		return 0;
	}
	/* convert first value */
	if ((adv = mpt_cdouble(&flt, val, 0)) <= 0) {
		errno = EINVAL;
		return 0;
	}
	len = strlen(val) + 1;
	
	if (!(iter = malloc(sizeof(*iter) + sizeof(*data) + len))) {
		return 0;
	}
	data = (void *) (iter + 1);
	
	iter->_vptr = &iteratorStr;
	
	memcpy(data + 1, val, len);
	data->next = val + adv;
	data->curr = flt;
	
	return iter;
}
