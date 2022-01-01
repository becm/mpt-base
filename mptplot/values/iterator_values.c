/*!
 * create iterator with double values read from string.
 */

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>

#include "convert.h"
#include "types.h"
#include "meta.h"

#include "values.h"

MPT_STRUCT(iteratorValues)
{
	MPT_INTERFACE(metatype) _mt;
	MPT_INTERFACE(iterator) _it;
	MPT_STRUCT(value) val;
	
	const char *next;  /* next position in string */
	double curr;       /* current iterator value */
};

/* convertable interface */
static int iterValueConv(MPT_INTERFACE(convertable) *val, int type, void *ptr)
{
	MPT_STRUCT(iteratorValues) *d = MPT_baseaddr(iteratorValues, val, _mt);
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
		return 's';
	}
	if (type == 's') {
		if (ptr) *((const char **) ptr) = (char *) d + 1;
		return MPT_ENUM(TypeIteratorPtr);
	}
	return MPT_ERROR(BadType);
}
/* metatype interface */
static void iterValueUnref(MPT_INTERFACE(metatype) *mt)
{
	free(mt);
}
static uintptr_t iterValueRef(MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
static MPT_INTERFACE(metatype) *iterValueClone(const MPT_INTERFACE(metatype) *mt)
{
	MPT_STRUCT(iteratorValues) *d = MPT_baseaddr(iteratorValues, mt, _mt);
	MPT_INTERFACE(metatype) *ptr;
	const char *values = (void *) (d + 1);
	
	if ((ptr = mpt_iterator_values(values))) {
		MPT_STRUCT(iteratorValues) *c = MPT_baseaddr(iteratorValues, ptr, _mt);
		size_t diff = d->next - values;
		c->next = ((const char *) (c + 1)) + diff;
		c->curr = d->curr;
	}
	return ptr;
}
/* iterator interface */
static const MPT_STRUCT(value) *iterValueValue(MPT_INTERFACE(iterator) *it)
{
	MPT_STRUCT(iteratorValues) *d = MPT_baseaddr(iteratorValues, it, _it);
	if (!d->next) {
		return 0;
	}
	d->val.domain = 0;
	d->val.type = 'd';
	d->val.ptr = memcpy(d->val._buf, &d->curr, sizeof(d->curr));
	
	return &d->val;
}
static int iterValueAdvance(MPT_INTERFACE(iterator) *it)
{
	MPT_STRUCT(iteratorValues) *d = MPT_baseaddr(iteratorValues, it, _it);
	int len;
	if (!d->next) {
		return MPT_ERROR(MissingData);
	}
	if (!*d->next || !(len = mpt_cdouble(&d->curr, d->next, 0))) {
		d->next = 0;
		return 0;
	}
	if (len < 0 || isnan(d->curr)) {
		return MPT_ERROR(BadValue);
	}
	d->next += len;
	return 'd';
}
static int iterValueReset(MPT_INTERFACE(iterator) *it)
{
	MPT_STRUCT(iteratorValues) *d = MPT_baseaddr(iteratorValues, it, _it);
	const char *base = (const char *) (d + 1);
	int len;
	if (!(len = mpt_cdouble(&d->curr, base, 0))) {
		d->curr = 0;
		return MPT_ERROR(MissingData);
	}
	if (len < 0 || isnan(d->curr)) {
		return MPT_ERROR(BadValue);
	}
	d->next = base + len;
	return 0;
}

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
extern MPT_INTERFACE(metatype) *mpt_iterator_values(const char *val)
{
	static const MPT_INTERFACE_VPTR(metatype) valueMeta = {
		{ iterValueConv },
		iterValueUnref,
		iterValueRef,
		iterValueClone
	};
	static const MPT_INTERFACE_VPTR(iterator) valueIter = {
		iterValueValue,
		iterValueAdvance,
		iterValueReset
	};
	MPT_STRUCT(iteratorValues) *data;
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
	
	if (!(data = malloc(sizeof(*data) + len))) {
		return 0;
	}
	data->_mt._vptr = &valueMeta;
	data->_it._vptr = &valueIter;
	
	data->val.domain = 0;
	data->val.type = 0;
	*((uint8_t *) &data->val._bufsize) = sizeof(data->val._buf);
	
	val = memcpy(data + 1, val, len);
	data->next = val + adv;
	data->curr = flt;
	
	return &data->_mt;
}
