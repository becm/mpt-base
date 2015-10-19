/*!
 * default axis values
 */

#ifndef _POSIX_C_SOURCE
# define _POSIX_C_SOURCE 200809L
#endif

#include <string.h>
#include <strings.h>
#include <stdlib.h>

#include "plot.h"

/*!
 * \ingroup mptPlot
 * \brief finalize axis
 * 
 * Clear resources of axis data.
 * 
 * \param axis axis data
 */
extern void mpt_axis_fini(MPT_STRUCT(axis) *axis)
{	
	free(axis->_title);
	axis->_title = 0;
}

/*!
 * \ingroup mptPlot
 * \brief initialize axis
 * 
 * Set default values for axis members.
 * 
 * \param axis uninitialized axis data
 * \param from axis data template
 */
extern void mpt_axis_init(MPT_STRUCT(axis) *axis, const MPT_STRUCT(axis) *from)
{
	if (from) {
		*axis = *from;
		
		if (axis->_title) {
			axis->_title = strdup(axis->_title);
		}
		return;
	}
	axis->_title = 0;
	
	axis->begin = 0.0;
	axis->end   = 1.0;
	
	axis->tlen = 0.3;
	
	axis->exp  = 0;

	axis->intv = 0;
	axis->sub = 0;
	
	axis->format = 0;
	axis->dec = 0;
	
	axis->lpos = axis ->tpos = 0;
}
