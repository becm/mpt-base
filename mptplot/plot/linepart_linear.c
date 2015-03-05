/*!
 * create visibility information
 */

#include <limits.h>

#include "plot.h"

/*!
 * \ingroup mptPlot
 * \brief get line part
 * 
 * Save data of next visible line part
 * assuming linear transformation.
 * Advance source part.raw elements to get
 * start address for next part.
 * 
 * \param part	line part information to fill
 * \param from	reference values
 * \param len	number of available values
 * \param range	allowed value range
 * 
 * \return encoded value
 */
extern void mpt_linepart_linear(MPT_STRUCT(linepart) *part, const double *from, size_t len, const MPT_STRUCT(dpoint) *range)
{
	double min, max;
	
	if (len > UINT16_MAX) len = UINT16_MAX;
	
	if (!range) {
		part->raw = part->usr = len;
		part->_cut = part->_trim = 0;
		return;
	}
	
	min = range->x;
	max = range->y;
	
	part->raw = part->usr = part->_cut = part->_trim = 0;
	
	if (!len) return;
	
	/* partial first */
	if (*from < min) {
		if (len >= 2 && from[1] >= min && from[1] <= max) {
			part->_cut = mpt_linepart_code((min-from[0])/(from[1]-from[0]));
			part->raw = part->usr = 2; from += 2; len -= 2;
		}
	}
	else if (*from > max) {
		if (len >= 2 && from[1] >= min && from[1] <= max) {
			part->_cut = mpt_linepart_code((from[0]-max)/(from[0]-from[1]));
			part->raw = part->usr = 2; from += 2; len -= 2;
		}
	}
	
	/* count visible points */
	while (len) {
		if (*from < min) {
			if (part->usr) {
				part->_trim = mpt_linepart_code((min-from[0])/(from[-1]-from[0]));
				++part->usr;
			}
			break;
		}
		if (*from > max) {
			if (part->usr) {
				part->_trim = mpt_linepart_code((from[0]-max)/(from[0]-from[-1]));
				++part->usr;
			}
			break;
		}
		part->raw = ++part->usr; ++from; --len;
	}
	if (!len) return;
	++from; --len;
	
	/* trailing invisible */
	while (len && (*from < min || *from > max)) {
		++part->raw; ++from; --len;
	}
	
	if (!len) ++part->raw;
}
