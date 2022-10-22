/*!
 * create iterator from string description.
 */

#include <errno.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

#include "types.h"
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
extern MPT_INTERFACE(metatype) *mpt_iterator_create(const char *conf)
{
	MPT_STRUCT(value) val = MPT_VALUE_INIT(0, 0);
	size_t len;
	char buf[32];
	char curr;
	
	if (!conf) {
		return _mpt_iterator_range(0);
	}
	/* consume whitespace */
	while (*conf && isspace(*conf)) {
		conf++;
	}
	if (!(curr = *conf)) {
		return _mpt_iterator_range(0);
	}
	/* extract description string */
	len = 0;
	while ((isupper(curr) || islower(curr))) {
		curr = conf[len++];
	}
	if (len >= sizeof(buf)) {
		errno = EINVAL;
		return 0;
	}
	/* no description -> use normal values */
	if (!len--) {
		return mpt_iterator_values(conf);
	}
	
	memcpy(buf, conf, len);
	buf[len] = 0;
	
	conf += len;
	MPT_value_set(&val, 's', &conf);
	
	/* create matching iterators */
	if (!strcasecmp(buf, "linear")
	 || !strcasecmp(buf, "lin")) {
		return _mpt_iterator_linear(&val);
	}
	if (!strcasecmp(buf, "factor")
	 || !strcasecmp(buf, "fact")
	 || !strcasecmp(buf, "fac")) {
		return _mpt_iterator_factor(&val);
	}
	if (!strcasecmp(buf, "range")) {
		return _mpt_iterator_range(&val);
	}
	/* unknown value iterator type */
	errno = EINVAL;
	return 0;
}

