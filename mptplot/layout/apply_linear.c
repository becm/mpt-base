/*!
 * transform double values to polyline
 */

#include <errno.h>

#include "layout.h"

/*!
 * \ingroup mptLayout
 * \brief apply value modifications
 * 
 * \param d	target points
 * \param pt	size and end info for data
 * \param src	data to apply
 * \param scale	modification for source data
 */
extern void mpt_apply_linear(MPT_STRUCT(dpoint) *d, const MPT_STRUCT(linepart) *pt, const double *src, const MPT_STRUCT(dpoint) *scale)
{
	size_t j = 0, len = pt->usr;
	double sx = 1, sy = 0;
	
	if (scale) { sx = scale->x; sy = scale->y; }
	
	if (pt->_cut) {
		double cut = mpt_linepart_real(pt->_cut);
		++j;
		cut = src[0] + cut * (src[1]-src[0]);
		
		d->x += sx * cut;
		d->y += sy * cut;
	}
	if (pt->_trim) {
		double trim = mpt_linepart_real(pt->_trim);
		--len;
		trim = src[len] - trim * (src[len]-src[len-1]);
		
		d[len].x += sx * trim;
		d[len].y += sy * trim;
	}
	for ( ; j < len; j++) {
		d[j].x += sx * src[j];
		d[j].y += sy * src[j];
	}
}
