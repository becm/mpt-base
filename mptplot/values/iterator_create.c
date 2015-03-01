/*!
 * create iterator from string description.
 */

#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <errno.h>

#include "values.h"


/*!
 * \ingroup mptValues
 * \brief create iterator
 * 
 * Create metatype supporting iterator properties.
 * 
 * \param conf  iterator description
 * 
 * \return interable metatype
 */
extern MPT_INTERFACE(iterator) *mpt_iterator_create(const char *conf)
{
	static const char delims[] = ": \t\r\n\f";
	const char *str = NULL, *delim = delims;
	
	if (!conf) {
		return _mpt_iterator_range("0 1");
	}
	while (*conf && isspace(*conf)) {
		conf++;
	}
	while (*delim) {
		if ((str = strchr(conf, *(delim++)))) {
			break;
		}
	}
	if (str) {
		if (!strncasecmp(conf, "lin", 3)) {
			return _mpt_iterator_linear(str+1);
		}
		if (!strncasecmp(conf, "fac", 3)) {
			return _mpt_iterator_factor(str+1);
		}
		if (!strncasecmp(conf, "val", 3)) {
			return _mpt_iterator_values(str+1);
		}
		if (!strncasecmp(conf, "ran", 3)) {
			return _mpt_iterator_range(str+1);
		}
	}
	return _mpt_iterator_range(conf);
}

