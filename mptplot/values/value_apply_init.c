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
extern void mpt_value_apply_init(MPT_STRUCT(value_apply) *va, int type)
{
	if (type < 0) {
		type = 0;
	}
	if (type & MPT_ENUM(AxisLimitSwap)) {
		va->scale = -1.0;
		va->add   =  1.0f;
	} else {
		va->scale = 1.0;
		va->add   = 0.0f;
	}
	switch (type & MPT_ENUM(AxisStyles)) {
	  case MPT_ENUM(AxisStyleX):
		va->to.x = 1;
		va->to.y = 0;
		break;
	  case MPT_ENUM(AxisStyleY):
		va->to.x = 0;
		va->to.y = 1;
		break;
	  case MPT_ENUM(AxisStyleZ):
		va->to.x = va->to.y = M_SQRT1_2;
		break;
	  default:
		va->to.x = va->to.y = 0;
	}
}
