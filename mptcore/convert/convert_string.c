
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
	int len = 0;
	if (!(txt = *from)) {
		return MPT_ERROR(BadArgument);
	}
	if ((type & 0xff) == MPT_ENUM(TypeValue)) {
		MPT_STRUCT(value) *val;
		
		if ((val = dest)) {
			val->fmt = 0;
			val->ptr = txt;
		}
		len = strlen(txt);
		if (type & MPT_ENUM(ValueConsume)) {
			*from = txt + len;
		}
		return len;
	}
	if ((type & 0xff) == 'k') {
		const char *tmp = *from;
		size_t klen;
		if (!(txt = mpt_convert_key(from, 0, &klen))) {
			return MPT_ERROR(BadValue);
		}
		/* restore start address */
		if (!(type & MPT_ENUM(ValueConsume))) {
			*from = tmp;
		}
		if (dest) ((const char **) dest)[0] = txt;
		return klen;
	}
	if ((type & 0xff) != 's') {
		while (isspace(*txt)) {
			++txt;
		}
		if ((len = mpt_convert_number(txt, type & 0xff, dest)) >= 0
		    && type & MPT_ENUM(ValueConsume)) {
			*from = txt + len;
		}
		return len;
	}
	if (dest) *(const char **) dest = txt;
	len = strlen(txt);
	if (type & MPT_ENUM(ValueConsume)) {
		*from = txt + len;
	}
	return len;
}
