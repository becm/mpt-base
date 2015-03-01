/*!
 * set values from string
 */

#include <stdlib.h>
#include <errno.h>

#include "values.h"

/*!
 * \ingroup mptOutput
 * \brief create value profile
 * 
 * Set data to values on string.
 * 
 * \param points number of elements to set
 * \param target address of elements to set
 * \param ld     element advance
 * \param param  text containing values
 * 
 * \return number of converted values
 */
extern int mpt_values_string(int max, double *target, int ld, const char *param)
{
	char	*end;
	int	points;
	
	if (max < 0) {
		errno = ERANGE; return -1;
	}
	if (!param) {
		return 0;
	}
	points = 0;
	
	while (points < max) {
		double	tmp = strtod(param, &end);
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

