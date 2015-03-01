/*!
 * set color attributes from string.
 */

#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <errno.h>

#include "layout.h"

static const struct {
	const char *name;
	MPT_STRUCT(color) c;
} named[] = {
	{ "black",   { 0xff, 0, 0, 0 } },
	
	{ "red",     { 0xff, 0xff, 0, 0 } },
	{ "green",   { 0xff, 0, 0xff, 0 } },
	{ "blue",    { 0xff, 0, 0, 0xff } },
	
	{ "cyan",    { 0xff, 0, 0xff, 0xff } },
	{ "yellow",  { 0xff, 0xff, 0xff, 0 } },
	{ "magenta", { 0xff, 0xff, 0, 0xff } },
	
	{ "white",   { 0xff, 0xff, 0xff, 0xff } }
};

/*!
 * \ingroup mptLayout
 * \brief parse color description
 * 
 * Simple name to color conversion
 * 
 * \param col  color data
 * \param txt  string containing color description
 * 
 * \return consumed length
 */
extern int mpt_color_parse(MPT_STRUCT(color) *color, const char *txt)
{
	int	len;
	
	if (!txt || !*txt) {
		if (color) mpt_color_set(color, 0, 0, 0);
		return 0;
	}
	if (!(*txt == '#')) {
		size_t i;
		
		for (i = 0; i < MPT_arrsize(named); i++) {
			len = strlen(named[i].name);
			if (!strncasecmp(named[i].name, txt, len) && (!txt[len] || isspace(txt[len]))) {
				if (color) *color = named[i].c;
				return len;
			}
		}
		errno = EINVAL;
		return -2;
	}
	len = mpt_color_html(color, txt+1);
	
	return len > 0 ? len+1 : len;
}
