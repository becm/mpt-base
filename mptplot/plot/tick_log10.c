/*!
 * logarithmic tick position.
 */

#include <math.h>
#include <errno.h>

#include "plot.h"

/*!
 * \ingroup mptPlot
 * \brief get tick position
 * 
 * Determine position of exponent value
 * 
 * \param val  exponent value [2..9]
 * 
 * \return position for exponent (0..1)
 */
extern double mpt_tick_log10(int val)
{
	static const double logs[8] = {
		0.3010299956639812,
		0.47712125471966244,
		0.6020599913279624,
		0.69897000433601886,
		0.77815125038364363,
		0.84509804001425681,
		0.90308998699194354,
		0.95424250943932487
	};
	if (val < 2 || val > 9) {
		return NAN;
	}
	return logs[val];
}
