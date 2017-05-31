
#include <limits.h>
#include <string.h>

#include "meta.h"

#include "layout.h"

/*!
 * \ingroup mptPlot
 * \brief set color data
 * 
 * Check range of color values and set if valid.
 * 
 * \param col   color data
 * \param red   red part [0..255]
 * \param green green part [0..255]
 * \param blue  blue part [0..255]
 * 
 * \return merged value
 */
extern int mpt_color_set(MPT_STRUCT(color) *col, int red, int green, int blue)
{
	if (red   < 0 || red   > UINT8_MAX) { return MPT_ERROR(BadValue); }
	if (green < 0 || green > UINT8_MAX) { return MPT_ERROR(BadValue); }
	if (blue  < 0 || blue  > UINT8_MAX) { return MPT_ERROR(BadValue); }
	
	col->red = red;
	col->green = green;
	col->blue = blue;
	col->alpha = 255;
	
	return (red * 0x10000) | (green * 0x100) | blue;
}
/*!
 * \ingroup mptPlot
 * \brief set color alpha
 * 
 * Check range of alpha values and set if valid.
 * 
 * \param col   color data
 * \param alpha red part [0..255]
 * 
 * \return new alha
 */
extern int mpt_color_setalpha(MPT_STRUCT(color) *col, int alpha)
{
	if (alpha < 0 || alpha > UINT8_MAX) { return MPT_ERROR(BadValue); }
	return col->alpha = alpha;
}
/*!
 * \ingroup mptPlot
 * \brief set color
 * 
 * Convert to color data
 * 
 * \param col  color data
 * \param src  data source to use
 * 
 * \return consumed length
 */
extern int mpt_color_pset(MPT_STRUCT(color) *col, const MPT_INTERFACE(metatype) *src)
{
	const char *txt;
	int len;
	
	if (!src) {
		/* default color is black, no transparency */
		static const MPT_STRUCT(color) tcol = MPT_COLOR_INIT;
		return memcmp(&tcol, col, sizeof(*col)) ? 1 : 0;
	}
	if ((len = src->_vptr->conv(src, MPT_ENUM(TypeColor), col)) >= 0) {
		return 0;
	}
	/* parse color name/format  */
	if ((len = src->_vptr->conv(src, 's', &txt)) >= 0) {
		int take;
		if ((take = mpt_color_parse(col, txt)) < 0) {
			return MPT_ERROR(BadValue);
		}
		return 0;
	}
	return MPT_ERROR(BadType);
}
