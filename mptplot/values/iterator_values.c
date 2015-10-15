/*!
 * create iterator with double values read from string.
 */

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>

#include "convert.h"

#include "values.h"

struct _iter_sdata
{
	MPT_INTERFACE(iterator) _it;
	const char *pos;  /* base/current position in string */
	double curr;      /* current iterator value */
};

static int iterUnref(MPT_INTERFACE(iterator) *iter)
{
	free(iter);
	return 0;
}
static double iterNext(MPT_INTERFACE(iterator) *iter, int step)
{
	struct _iter_sdata *it = (void *) iter;
	const char *desc;
	double curr;
	int len;
	
	if (step < 0) {
		desc = (const char *) (it + 1);
		if ((len = mpt_cdouble(&curr, desc, 0)) <= 0) {
			return NAN;
		}
		it->pos = desc + len;
		return it->curr = curr;
	}
	if (!step) {
		return it->curr;
	}
	desc = it->pos;
	do {
		if ((len = mpt_cdouble(&curr, desc, 0)) <= 0) {
			return NAN;
		}
		desc += len;
	} while (--step);
	
	it->pos = desc;
	return it->curr = curr;
}

static MPT_INTERFACE_VPTR(iterator) ictl = { iterUnref, iterNext };

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
extern MPT_INTERFACE(iterator) *_mpt_iterator_values(const char *conf)
{
	struct _iter_sdata *it;
	size_t len;
	double val;
	int adv;
	
	if (!conf) {
		errno = EFAULT;
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
	
	it->_it._vptr = &ictl;
	it->pos += adv;
	it->curr = val;
	
	return &it->_it;
}
