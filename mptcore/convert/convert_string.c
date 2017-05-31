
#include <ctype.h>
#include <string.h>

#include "meta.h"

#include "convert.h"

/*!
 * \ingroup mptConvert
 * \brief get data from string
 * 
 * convert string data to specified type
 * 
 * \param from pointer to string data
 * \param type convertion target type
 * \param dest target data address
 * 
 * \return length of converted type
 */
extern int mpt_convert_string(const char *from, int type, void *dest)
{
	int len = 0;
	
	if (type == MPT_ENUM(TypeValue)) {
		MPT_STRUCT(value) *val;
		
		if ((val = dest)) {
			val->fmt = 0;
			val->ptr = from;
		}
		return strlen(from);
	}
	if (type == 'k') {
		const char *key, *txt = from;
		size_t klen;
		if (!(key = mpt_convert_key(&txt, 0, &klen))) {
			return MPT_ERROR(BadValue);
		}
		if (dest) {
			((const char **) dest)[0] = txt;
		}
		/* restore start address */
		return txt - from;
	}
	if (type != 's') {
		const char *txt = from;
		while (isspace(*txt)) {
			++txt;
		}
		if ((len = mpt_convert_number(txt, type, dest)) < 0) {
			return len;
		}
		txt += len;
		return txt - from;
	}
	if (dest) {
		*(const char **) dest = from;
	}
	return strlen(from);
}
