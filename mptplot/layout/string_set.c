/*!
 * set text string pointer data.
 */

#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "layout.h"

/*!
 * \ingroup mptLayout
 * \brief change text
 * 
 * Set character data.
 * 
 * \param ptr  string pointer reference
 * \param data data source to change string
 * \param len  new data length
 * 
 * \return new data length
 */
extern int mpt_string_set(char **ptr, const char *data, int len)
{
	char *txt;
	
	if (!data) len = 0;
	else if (len < 0) len = strlen(data);
	
	if (!len) { free(*ptr); *ptr = 0; return 0; }
	
	if (data == *ptr) return 0;
	
	if (!(txt = realloc(*ptr, len+1))) return -1;
	
	*ptr = memcpy(txt, data, len);
	txt[len] = 0;
	
	return len;
}

/*!
 * \ingroup mptLayout
 * \brief change text
 * 
 * Get/Set character data.
 * 
 * \param ptr  string pointer reference
 * \param src  data source to change string
 * 
 * \return consumed/changed value
 */
extern int mpt_string_pset(char **ptr, MPT_INTERFACE(metatype) *src)
{
	char *txt;
	int len;
	
	if (!src) return (ptr && *ptr) ? strlen(*ptr) : 0;
	if ((len = src->_vptr->conv(src, 's', &txt)) < 0) return len;
	return mpt_string_set(ptr, txt, len ? len : -1);
}
