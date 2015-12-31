/*!
 * generic metatype info
 */

#include <errno.h>
#include <string.h>
#include <strings.h>

#include <sys/uio.h>

#include "convert.h"

#include "core.h"

struct metaInfo {
	uint16_t size, /* free allocated memory */
	         used; /* used data size */
	uint32_t line;
};
static int geninfoSet(struct metaInfo *info, const char *val, int len)
{
	if (len < 0) {
		if (!val) {
			info->used = 0;
			return 0;
		}
		len = strlen(val);
	}
	/* remove tailing data */
	else if (!val) {
		if (len >= info->used) {
			return MPT_ERROR(BadArgument);
		}
		((char *) (info+1))[len] = 0;
		info->used = len + 1;
		
		return 2;
	}
	if (len >= info->size) {
		return MPT_ERROR(MissingBuffer);
	}
	memcpy(info+1, val, len);
	((char *) (info+1))[len] = 0;
	
	info->used = len + 1;
	
	return len;
}
/*!
 * \brief get/set geninfo value
 * 
 * Set value of geninfo data.
 * 
 * \param raw  start of geninfo data
 * \param prop property to change/request
 * \param src  data source to change property
 * 
 * \return >= 0 on success
 */
extern int _mpt_geninfo_value(uint64_t *raw, const MPT_STRUCT(value) *val)
{
	struct metaInfo *info = (void *) raw;
	const void *ptr;
	const char *base;
	size_t len;
	uint32_t line;
	int ret;
	
	if (!val) {
		return info->used ? info->used - 1 : MPT_ERROR(BadValue);
	}
	if (!val->fmt) {
		return geninfoSet(info, val->ptr, -1);
	}
	if (!*val->fmt) {
		return MPT_ERROR(BadValue);
	}
	/* try string format */
	ptr = val->ptr;
	if (!(base = mpt_data_tostring(&ptr, val->fmt[0], &len))) {
		/* require string or integer */
		if ((ret = mpt_data_convert(&ptr, val->fmt[0], &line, 'u')) < 0) {
			return MPT_ERROR(BadType);
		}
		ret = 1;
		if (val->fmt[1]) {
			/* too much data */
			if (val->fmt[2]) {
				return MPT_ERROR(BadValue);
			}
			/* second value must be string */
			if (!(base = mpt_data_tostring(&ptr, val->fmt[1], &len))) {
				return MPT_ERROR(BadValue);
			}
			if ((ret = geninfoSet(info, base, len)) < 0) {
				return ret;
			}
			ret = 2;
		}
		info->line = line;
		return ret;
	}
	if (val->fmt[1]) {
		/* too much data */
		if (val->fmt[2]) {
			return MPT_ERROR(BadValue);
		}
		/* require integer */
		if (mpt_data_convert(&ptr, val->fmt[0], &line, 'u') < 0) {
			return MPT_ERROR(BadType);
		}
		info->line = line;
		
		return 1;
	}
	return 0;
}
/*!
 * \brief initialize geninfo data
 * 
 * set property of geninfo data
 * 
 * \param raw   start of geninfo data
 * \param dlen  needed data length
 * \param ilen  identifier length (<0 for non-printable)
 * \param ident identifier data
 * \param ref   initial reference count
 * 
 * \return >= 0 on success
 */
extern int _mpt_geninfo_init(void *raw, size_t dlen)
{
	struct metaInfo *info = raw;
	
	if (dlen < sizeof(*info)) {
		return -2;
	}
	dlen -= sizeof(*info);
	if (dlen > UINT16_MAX) {
		dlen = UINT16_MAX;
	}
	info->size = dlen;
	info->used = 0;
	info->line = 0;
	
	return dlen;
}

