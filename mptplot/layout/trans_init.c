/*!
 * set dimension transform
 */

#include <math.h>

#include "layout.h"

/* no separate #define magic to get universal constant... */
#ifndef M_SQRT1_2
# define M_SQRT1_2 0.70710678118654752440
#endif

/*!
 * \ingroup mptPlot
 * \brief axis transformation
 * 
 * Set default axis type transformation.
 * 
 * \param tr   dimension transform data
 * \param type axis/dimension type
 */
extern void mpt_trans_init(MPT_STRUCT(transform) *tr, enum MPT_ENUM(AxisFlag) type)
{
	tr->move.x  = tr->move.y  = 0;
	
	switch (type) {
	  case MPT_ENUM(AxisStyleX): tr->scale.x = 1; tr->scale.y = 0; break;
	  case MPT_ENUM(AxisStyleY): tr->scale.x = 0; tr->scale.y = 1; break;
	  case MPT_ENUM(AxisStyleZ): tr->scale.x = tr->scale.y = M_SQRT1_2; break;
	  default: tr->scale.x = tr->scale.y = 0;
	}
	tr->min = 0;
	tr->max = 1;
}
