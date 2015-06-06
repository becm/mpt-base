/*!
 * advance needed iterator steps to guarantee iter.curr() > last.
 */

#include <math.h>
#include <errno.h>

#include "values.h"


/*!
 * \ingroup mptValues
 * \brief current value
 * 
 * Get current iterator value.
 * 
 * \param iter  iterator descriptor
 * 
 * \return current metatype value
 */
extern double mpt_iterator_curr(MPT_INTERFACE(iterator) *it)
{
	return it->_vptr->next(it, 0);
}

/*!
 * \ingroup mptValues
 * \brief advance iterator
 * 
 * Get next iterator value after passed position.
 * Advance iterator until condition is achieved.
 * 
 * \param      iter  iterator descriptor
 * \param[in]  last  current position
 * \param[out] last  next iterator value
 * 
 * \return iterator advancements
 */
extern int mpt_iterator_next(MPT_INTERFACE(iterator) *it, double *last)
{
	double curr, ref;
	int steps = 0;
	
	if (!last) {
		curr = it->_vptr->next(it, 1);
		return isnan(curr) ? 0 : 1;
	}
	curr = it->_vptr->next(it, 0);
	ref = *last;
	while (curr <= ref) {
		double	save = curr;
		
		curr = it->_vptr->next(it, 1);
		
		if (isnan(curr)) {
			return -2;
		}
		if (curr <= save) {
			errno = ERANGE;
			return -2;
		}
		++steps;
	}
	*last = curr;
	return steps;
}
