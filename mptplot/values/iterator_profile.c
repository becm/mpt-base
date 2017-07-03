/*!
 * select data type from description
 */

#include <string.h>
#include <strings.h>
#include <ctype.h>

#include <errno.h>
#include <fcntl.h>

#include "convert.h"

#include "values.h"

static const char *nextVis(const char *ptr, const char *cont)
{
	if (cont) {
		while (*cont) {
			if (!*ptr || (isspace(*ptr) || *ptr == ':')) {
				break;
			}
			if (*ptr++ != *cont++) {
				return 0;
			}
		}
	}
	else {
		if (!*ptr) {
			return ptr;
		}
		if (!isspace(*ptr) && *ptr != ':') {
			return 0;
		}
		++ptr;
	}
	while (*ptr && isspace(*ptr)) ++ptr;
	if (*ptr && *ptr == ':') {
		++ptr;
		while (*ptr && isspace(*ptr)) ++ptr;
	}
	return ptr;
}

static int getValues(double *val, int len, const char *ptr)
{
	int i = 0;
	while (i < len) {
		ssize_t len = mpt_cdouble(val + i, ptr, 0);
		if (len <= 0) {
			return i;
		}
		ptr += len;
		++i;
	}
	return i;
}
/*!
 * \ingroup mptValues
 * \brief profile type
 * 
 * Get type of profile to generate.
 * 
 * \param[in]  start  profile description
 * \param[out] end    end of consumed string
 * 
 * \return type of profile
 */
extern MPT_INTERFACE(iterator) *mpt_iterator_profile(const _MPT_ARRAY_TYPE(double) *arr, const char *desc)
{
	MPT_STRUCT(buffer) *buf;
	long len;
	int type, match;
	
	if (!(buf = arr->_buf) || !(len = buf->_used / sizeof(double))) {
		errno = EINVAL;
		return 0;
	}
	if ((type = buf->_vptr->content(buf)) && type != 'd') {
		errno = EINVAL;
		return 0;
	}
	if (!desc) {
		return mpt_iterator_values(0);
	}
	while (*desc && isspace(*desc)) {
		desc++;
	}
	if (!strncasecmp(desc, "lin", match = 3)) {
		double tmp[2];
		if (!(desc = nextVis(desc + match, "ear")) || getValues(tmp, 2, desc) < 2) {
			errno = EINVAL;
			return 0;
		}
		return mpt_iterator_linear(len, tmp[0], tmp[1]);
	}
	if (!strncasecmp(desc, "bound", match = 5)) {
		double tmp[3];
		if (!(desc = nextVis(desc + match, "ary")) || getValues(tmp, 3, desc) < 3) {
			errno = EINVAL;
			return 0;
		}
		return mpt_iterator_boundary(len, tmp[0], tmp[1], tmp[2]);
	}
	if (!strncasecmp(desc, "poly", match = 4)) {
		if (!(desc = nextVis(desc + match, 0))) {
			errno = EINVAL;
			return 0;
		}
		return mpt_iterator_poly(desc, arr);
	}
	if (!strncasecmp(desc, "file", match = 4)) {
		int fd;
		if (!(desc = nextVis(desc + match, 0))) {
			errno = EINVAL;
			return 0;
		}
		if ((fd = open(desc, O_RDONLY)) < 0) {
			return 0;
		}
		return mpt_iterator_file(fd);
	}
	errno = EINVAL;
	return 0;
}

