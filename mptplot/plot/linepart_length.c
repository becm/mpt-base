
#include "plot.h"

/*!
 * \ingroup mptPlot
 * \brief lineparts user point count
 * 
 * Determine user point count needed for line parts.
 * 
 * \param pt  line part base address
 * \param seg number of line parts
 * 
 * \return total user point count
 */
extern size_t mpt_linepart_ulength(const MPT_STRUCT(linepart) *pt, size_t seg)
{
	size_t len = 0;
	while (seg--) len += (pt++)->usr;
	return len;
}
/*!
 * \ingroup mptPlot
 * \brief lineparts raw point count
 * 
 * Determine raw point count of line parts.
 * 
 * \param pt  line part base address
 * \param seg number of line parts
 * 
 * \return total raw point count
 */
extern size_t mpt_linepart_rlength(const MPT_STRUCT(linepart) *pt, size_t seg)
{
	size_t len = 0;
	while (seg--) len += (pt++)->raw;
	return len;
}
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
