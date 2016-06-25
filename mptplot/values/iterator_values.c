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
	MPT_INTERFACE(metatype) mt;
	const char *pos;  /* base/current position in string */
	double curr;      /* current iterator value */
};

static void iterUnref(MPT_INTERFACE(metatype) *mt)
{
	free(mt);
}
static int iterAssign(MPT_INTERFACE(metatype) *mt, const MPT_STRUCT(value) *val)
{
	struct _iter_sdata *it = (void *) mt;
	const char *pos;
	int len;
	
	if (val) {
		return MPT_ERROR(BadOperation);
	}
	pos = (const char *) (it+1);
	
	if ((len = mpt_cdouble(&it->curr, pos, 0)) < 0) {
		return MPT_ERROR(BadValue);
	}
	it->pos = pos + len;
	
	return 0;
}
static int iterConv(MPT_INTERFACE(metatype) *mt, int t, void *ptr)
{
	struct _iter_sdata *it = (void *) mt;
	if (!t) {
		static const char fmt[] = { 'd', 0 };
		if (ptr) *((const char **) ptr) = fmt;
		return 0;
	}
	if ((t & 0xff) != 'd') {
		return MPT_ERROR(BadType);
	}
	if (!it->pos) {
		return 0;
	}
	if (ptr) {
		*((double *) ptr) = it->curr;
	}
	if (t & MPT_ENUM(ValueConsume)) {
		int len;
		if ((len = mpt_cdouble(&it->curr, it->pos, 0)) <= 0 || isnan(it->curr)) {
			it->pos = 0;
		} else {
			it->pos += len;
		}
		return 'd' | MPT_ENUM(ValueConsume);
	}
	return 'd';
}
static const MPT_INTERFACE_VPTR(metatype) iteratorValues;
static MPT_INTERFACE(metatype) *iterClone(const MPT_INTERFACE(metatype) *mt)
{
	struct _iter_sdata *it = (void *) mt;
	const char *val;
	size_t len, pos;
	double curr;
	
	curr = it->curr;
	val = (const char *) (it+1);
	pos = it->pos - val;
	len = strlen(val);
	
	if (!(it = malloc(sizeof(*it) + len + 1))) {
		return 0;
	}
	val = memcpy(it+1, val, len+1);
	
	it->mt._vptr = &iteratorValues;
	it->pos = val + pos;
	it->curr = curr;
	
	return memcpy(it, mt, sizeof(*it));
}

static const MPT_INTERFACE_VPTR(metatype) iteratorValues = {
	iterUnref,
	iterAssign,
	iterConv,
	iterClone
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
extern MPT_INTERFACE(metatype) *_mpt_iterator_values(const char *conf)
{
	struct _iter_sdata *it;
	size_t len;
	double val;
	int adv;
	
	if (!conf) {
		return 0;
	}
	/* convert first value */
	if ((adv = mpt_cdouble(&val, conf, 0)) <= 0 || isnan(val)) {
		return 0;
	}
	len = strlen(conf) + 1;
	
	if (!(it = malloc(sizeof(*it) + len))) {
		return 0;
	}
	it->pos = memcpy(it+1, conf, len);
	
	it->mt._vptr = &iteratorValues;
	it->pos += adv;
	it->curr = val;
	
	return &it->mt;
}
