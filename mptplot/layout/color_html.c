/*!
 * set color attributes from string.
 */

#include <errno.h>

#include "convert.h"
#include "layout.h"

/*!
 * \ingroup mptLayout
 * \brief parse color value
 * 
 * Convert text to color data
 * 
 * \param col	color data
 * \param txt	string containing hex color value
 * 
 * \return consumed length
 */
extern int mpt_color_html(MPT_STRUCT(color) *color, const char *txt)
{
	uint8_t	col[4] = { 0, 0, 0, 255 };
	char	part[4];
	int	len, i = 0;
	
	if (!txt) {
		errno = EINVAL; return -2;
	}
	part[2] = '\0';
	part[3] = 4;
	len = 0;
	
	while (part[3]--) {
		if (!(part[0] = txt[len++]))
			break;
		
		if (!(part[1] = txt[len++])) {
			errno = EINVAL; return -1;
		}
		if (mpt_cuint8(&col[i++], part, 0x10, 0) < 0)
			return -1;
	}
	if (color) {
		mpt_color_set(color, col[0], col[1], col[2]);
		mpt_color_setalpha(color, col[3]);
	}
	return len;
}
