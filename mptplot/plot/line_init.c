
#include "plot.h"

/*!
 * \ingroup mptPlot
 * \brief initialize line structure
 * 
 * Set default values for line members.
 * 
 * \param line  uninitialized line data
 */
extern void mpt_line_init(MPT_STRUCT(line) *line)
{
	mpt_color_set(&line->color, 0, 0, 0);
	mpt_lattr_set(&line->attr, 1, 1, 0, 10);
	
	line->from.x = line->from.y = 0.0;
	line->to.x = line->to.y = 0.0;
}
