/*!
 * create equidistant ticks.
 */

#include <errno.h>

#include "layout.h"

/*!
 * \ingroup mptPlot
 * \brief set linear ticks
 * 
 * Set ticks with total delta relative to first.
 * Tick difference also copied from first.
 * 
 * Each tick requires two points!
 * 
 * \param pts  points to generate relative tick data on
 * \param nt   number ot ticks
 * \param dx   total x delta
 * \param dy   toty y delta
 */
extern void mpt_ticks_linear(MPT_STRUCT(dpoint) *pts, size_t nt, double dx, double dy)
{
	double bx, by, tx, ty;
	size_t i;
	
	if (!nt--) {
		return;
	}
	/* tick base */
	bx = pts[0].x;
	by = pts[0].y;
	/* tick end */
	tx = pts[1].x;
	ty = pts[1].y;
	
	/* tick difference */
	dx = dx / nt;
	dy = dy / nt;
	
	/* set tick data */
	for (i = 1; i <= nt; i++) {
		double x = i * dx, y = i * dy;
		
		pts[0].x = bx + x;
		pts[0].y = by + y;
		pts[1].x = tx + x;
		pts[1].y = ty + y;
		
		pts += 2;
	}
}

