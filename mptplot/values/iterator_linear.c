/*!
 * create iterator with constant difference between elements.
 */

#include <stdlib.h>
#include <errno.h>
#include <math.h>

#include "convert.h"

#include "values.h"

struct _iter_ldata
{
	MPT_INTERFACE(iterator) _it;
	double base,  /* current/base value */
	       step;  /* advance step size */
	int pos, max; /* curr = first + step * pos */
};

static int iterUnref(MPT_INTERFACE(iterator) *iter)
{
	free(iter);
	return 0;
}
static double iterNext(MPT_INTERFACE(iterator) *iter, int step)
{
	struct _iter_ldata *it = (void *) iter;
	
	step = (step < 0) ? 0 : (it->pos + step);
	
	if (step > it->max) {
		return NAN;
	}
	return it->base + (it->pos = step) * it->step;
}
static MPT_INTERFACE_VPTR(iterator) ictl = { iterUnref, iterNext };

/*!
 * \ingroup mptValues
 * \brief create linear iterator
 * 
 * Create iterator with linear advancing.
 * 
 * \param conf  linear iterator parameters
 * 
 * \return iterator interface
 */
extern MPT_INTERFACE(iterator) *_mpt_iterator_linear(const char *conf)
{
	struct _iter_ldata *it;
	double base = 0.0, step = 0.1;
	int max = 10, len;
	
	if (!conf) {
		errno = EFAULT; return 0;
	}
	/* try to get initial value */
	if ((len = mpt_cdouble(&base, conf, 0)) <= 0)
		return 0;
	
	/* step size */
	if ((len = mpt_cdouble(&step, conf += len, 0)) <= 0) {
		step = base; base = 0.0;
	}
	/* iterations */
	else if ((len = mpt_cint(&max, conf += len, 0, 0)) <= 0) {
		max = 10;
	}
	if (!(it = malloc(sizeof(*it)))) {
		return 0;
	}
	it->_it._vptr = &ictl;
	it->base = base;
	it->step = step;
	it->pos  = 0;
	it->max  = max;
	
	return &it->_it;
}

/*!
 * \ingroup mptValues
 * \brief create range iterator
 * 
 * Create iterator with linear advancing.
 * 
 * \param conf	range iterator parameters
 * 
 * \return iterator interface
 */
extern MPT_INTERFACE(iterator) *_mpt_iterator_range(const char *conf)
{
	MPT_INTERFACE(iterator) *iter;
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

