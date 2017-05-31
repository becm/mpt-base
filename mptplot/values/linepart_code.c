
#include "values.h"

/*!
 * \ingroup mptPlot
 * \brief line part trim/cut encoding
 * 
 * Calculate encoded cut/trim value
 * 
 * \param val  value of range [0,1)
 * 
 * \return encoded value
 */
extern int mpt_linepart_code(double val)
{
	double	small;
	if (val < 0 || val > 1) return -2;
	small = val * (UINT16_MAX + 1);
	if (val && !small) return 1;
	return (small > UINT16_MAX) ? UINT16_MAX : small;
}
/*!
 * \ingroup mptPlot
 * \brief line part trim/cut decoding
 * 
 * Calculate decoded cut/trim value
 * 
 * \param val  encoded value
 * 
 * \return decoded value of range [0,1)
 */
extern double mpt_linepart_real(int val)
{
	return ((double) val) / (UINT16_MAX + 1);
}
