
#include <ctype.h>
#include <string.h>

#include "convert.h"

/*!
 * \ingroup mptConvert
 * \brief get data from string
 * 
 * convert string data to specified type
 * 
 * \param from pointer to current string position
 * \param type convertion target type
 * \param dest target data address
 * 
 * \return length of converted type
 */
extern int mpt_convert_string(const char **from, int type, void *dest)
{
	const char *txt;
	int	len = 0;
	if (!(txt = *from)) return -1;
	if (type == 'k') {
		size_t	klen;
		if (!(txt = mpt_convert_key(from, 0, &klen))) return -2;
		if (dest) ((const char **) dest)[0] = txt;
		return klen;
	}
	if (type != 's') {
		if ((len = mpt_convert(txt, type, dest)) > 0) *from = txt + len;
		return len;
	}
	while (*txt && isspace(*txt)) {
		++txt; ++from;
	}
	if (dest) *(const char **) dest = txt;
	len = strlen(txt);
	*from = txt + len;
	return len;
}
