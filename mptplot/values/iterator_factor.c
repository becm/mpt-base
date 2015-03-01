/*!
 * create iterator with constant faktor between elements.
 */

#include <stdlib.h>
#include <errno.h>
#include <math.h>

#include "convert.h"

#include "values.h"

struct _iter_fdata
{
	MPT_INTERFACE(iterator) _it;
	double curr, base, /* current/base value */
	       fact;       /* multiplication factor */
	int    pos, max;   /* iterations left */
};

static int iterUnref(MPT_INTERFACE(iterator) *iter)
{
	free(iter);
	return 0;
}

static double iterNext(MPT_INTERFACE(iterator) *iter, int step)
{
	struct _iter_fdata *it = (void *) iter;
	
	if (step < 0) {
		step = it->pos = 0;
		return it->curr = it->base;
	}
	if (step + it->pos >= it->max) {
		return NAN;
	}
	else {
		double curr = it->curr, fact = it->fact;
		
		while (step--) {
			curr = curr ? curr * fact : fact;
			++it->pos;
		}
		return it->curr = curr;
	}
}

static MPT_INTERFACE_VPTR(iterator) ictl = { iterUnref, iterNext };

/*!
 * \ingroup mptValues
 * \brief create factor iterator
 * 
 * Create iterator advancing by factor.
 * 
 * \param conf	factor iterator parameters
 * 
 * \return iterator interface
 */
extern MPT_INTERFACE(iterator) *_mpt_iterator_factor(const char *conf)
{
	struct _iter_fdata *it;
	double	base = 1.0, fact = 10.;
	int	max = 10, len;
	
	if (!conf) {
		errno = EFAULT;
		return 0;
	}
	/* try to get initial value */
	if ((len = mpt_cdouble(&base, conf, 0)) <= 0)
		return 0;
	
	if ((len = mpt_cdouble(&fact, conf += len, 0)) <= 0)
		fact = 10.0;
	
	else if ((len = mpt_cint(&max, conf += len, 0, 0)) <= 0)
		max = 10;
	
	if (!(it = malloc(sizeof(*it)))) {
		return 0;
	}
	it->_it._vptr = &ictl;
	it->base = base;
	it->curr = 0.0;
	it->fact = fact;
	it->pos  = 0;
	it->max  = max;
	
	return &it->_it;
}

