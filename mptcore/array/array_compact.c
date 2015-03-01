/*!
 * compact array
 */

#include "array.h"

/*!
 * \ingroup mptArray
 * \brief compact pointers
 * 
 * Move non-zero pointers to beginning of array
 * while preserving order.
 * 
 * \param base	array containing pointers
 * \param len	number of elements in array
 * 
 * \return used elements
 */
extern size_t mpt_array_compact(void **base, size_t len)
{
	void	**to, **end;
	size_t	rem;
	
	/* loop borders */
	end = base + len;
	
	for (to = 0, rem = 0; base < end; ++base) {
		/* save available space */
		if (!*base) {
			if (!to) {
				to = base;
			}
			continue;
		}
		++rem;
		
		/* no smaller position available */
		if (!to) {
			continue;
		}
		*to = *base;
		*base = 0;
		
		/* find smallest free position */
		while (++to < base) {
			if (!*(to)) break;
		}
	}
	return rem;
}
