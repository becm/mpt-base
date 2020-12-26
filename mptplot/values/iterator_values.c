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

struct _iter_sdata
{
	const char *next;  /* next position in string */
	double curr;       /* current iterator value */
};

/* convertable interface */
static int iterValueConv(MPT_INTERFACE(convertable) *val, int type, void *ptr)
{
	const struct _iter_sdata *d = (void *) (val + 2);
	if (!type) {
		static const uint8_t fmt[] = { MPT_ENUM(TypeIteratorPtr), 0 };
		if (ptr) {
			*((const uint8_t **) ptr) = fmt;
			return 'd';
		}
		return MPT_ENUM(TypeIteratorPtr);
	}
	if (type == MPT_ENUM(TypeIteratorPtr)) {
		if (ptr) *((const void **) ptr) = val + 1;
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
	struct _iter_sdata *d = (void *) (mt + 2);
	MPT_INTERFACE(metatype) *ptr;
	
	if ((ptr = mpt_iterator_values((void *) (d + 1)))) {
		struct _iter_sdata *val = (void *) (ptr + 2);
		size_t diff = d->next - (char *) mt;
		val->next = ((char *) mt) + diff;
		val->curr = d->curr;
	}
	return ptr;
}
/* iterator interface */
static int iterValueGet(MPT_INTERFACE(iterator) *it, int type, void *ptr)
{
	struct _iter_sdata *d = (void *) (it + 1);
	if (!type) {
		static const uint8_t fmt[] = "df";
		if (ptr) {
			*((const uint8_t **) ptr) = fmt;
		}
		return d->next - (char *) (d + 1);
	}
	if (!d->next) {
		return 0;
	}
	if (isnan(d->curr)) {
		return MPT_ERROR(BadValue);
	}
	if (type == 'd') {
		if (ptr) *((double *) ptr) = d->curr;
		return 's';
	}
	if (type == 'f') {
		if (ptr) *((float *) ptr) = d->curr;
		return 's';
	}
	return MPT_ERROR(BadType);
}
static int iterValueAdvance(MPT_INTERFACE(iterator) *it)
{
	struct _iter_sdata *d = (void *) (it + 1);
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
	struct _iter_sdata *d = (void *) (it + 1);
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
		iterValueGet,
		iterValueAdvance,
		iterValueReset
	};
	MPT_INTERFACE(metatype) *mt;
	MPT_INTERFACE(iterator) *it;
	struct _iter_sdata *d;
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
	
	if (!(mt = malloc(sizeof(*mt) + sizeof(*it) + sizeof(*d) + len))) {
		return 0;
	}
	mt->_vptr = &valueMeta;
	
	it = (void *) (mt + 1);
	it->_vptr = &valueIter;
	
	d = (void *) (it + 1);
	val = memcpy(d + 1, val, len);
	d->next = val + adv;
	d->curr = flt;
	
	return mt;
}
