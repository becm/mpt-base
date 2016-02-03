/*!
 * set values from string
 */

#include <stdlib.h>
#include <errno.h>

#include "values.h"

/*!
 * \ingroup mptValues
 * \brief create value profile
 * 
 * Set data to values on string.
 * 
 * \param param  text containing values
 * \param points number of elements to set
 * \param target address of elements to set
 * \param ld     element advance
 * 
 * \return number of converted values
 */
extern int mpt_values_string(const char *param, int max, double *target, int ld)
{
	char *end;
	int points;
	
	if (max < 0) {
		return MPT_ERROR(BadArgument);
	}
	if (!param) {
		return 0;
	}
	points = 0;
	
	while (points < max) {
		double tmp = strtod(param, &end);
		if (param == end) {
			break;
		}
		param = end;
		*target = tmp;
		target += ld;
		points++;
	}
	while (points < max--) {
		*target = 0.;
		target += ld;
	}
	return points;
}

