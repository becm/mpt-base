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
extern void mpt_trans_init(MPT_STRUCT(transform) *tr, int type)
{
	if (type < 0) {
		type = 0;
	}
	if (type & MPT_ENUM(AxisLimitSwap)) {
		tr->scale = -1.0;
		tr->add   =  1.0f;
	} else {
		tr->scale = 1.0;
		tr->add   = 0.0f;
	}
	switch (type & MPT_ENUM(AxisStyles)) {
	  case MPT_ENUM(AxisStyleX): tr->apply.x = 1; tr->apply.y = 0; break;
	  case MPT_ENUM(AxisStyleY): tr->apply.x = 0; tr->apply.y = 1; break;
	  case MPT_ENUM(AxisStyleZ): tr->apply.x = tr->apply.y = M_SQRT1_2; break;
	  default: tr->apply.x = tr->apply.y = 0;
	}
}
