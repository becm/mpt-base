/*!
 * create iterator from string description.
 */

#include <string.h>
#include <strings.h>
#include <ctype.h>

#include "convert.h"

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
	static const MPT_STRUCT(value) def = { 0, "0 1" };
	static const char delims[] = ": \t\r\n\f";
	const char *delim = delims;
	MPT_STRUCT(value) tmp;
	
	if (!conf) {
		return _mpt_iterator_range(def);
	}
	while (*conf && isspace(*conf)) {
		conf++;
	}
	tmp.fmt = 0;
	while (*delim) {
		if ((tmp.ptr = strchr(conf, *(delim++)))) {
			break;
		}
	}
	if (!tmp.ptr) {
		tmp.ptr = conf;
		return _mpt_iterator_range(tmp);
	}
	tmp.ptr = ((const char *) tmp.ptr) + 1;
	if (!strncasecmp(conf, "lin", 3)) {
		return _mpt_iterator_linear(tmp);
	}
	if (!strncasecmp(conf, "fac", 3)) {
		return _mpt_iterator_factor(tmp);
	}
	if (!strncasecmp(conf, "val", 3)) {
		return _mpt_iterator_values(tmp);
	}
	if (!strncasecmp(conf, "ran", 3)) {
		return _mpt_iterator_range(tmp);
	}
	/* not a valid format */
	if (mpt_cdouble(0, conf, 0) < 0) {
		return 0;
	}
	/* fallback to range */
	tmp.ptr = conf;
	return _mpt_iterator_range(tmp);
}

