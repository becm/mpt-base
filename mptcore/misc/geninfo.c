/*!
 * generic metatype info
 */

#include <errno.h>
#include <string.h>

#include <limits.h>
#include <sys/uio.h>

#include "meta.h"
#include "convert.h"

#include "core.h"

struct metaInfo {
	uint16_t size, /* free allocated memory */
	         used; /* used data size */
};
/*!
 * \brief get/set geninfo value
 * 
 * Set value of geninfo data.
 * 
 * \param raw  start of geninfo data
 * \param src  text data to assign
 * \param len  new text length
 * 
 * \return >= 0 on success
 */
int _mpt_geninfo_size(size_t post)
{
	size_t align;
	if (post > INT_MAX) {
		return MPT_ERROR(BadValue);
	}
	post += sizeof(struct metaInfo) + 1;
	if (post > UINT8_MAX) {
		return MPT_ERROR(MissingBuffer);
	}
	align = MPT_align(post);
	if (align < UINT8_MAX) {
		return align;
	}
	return post;
}
/*!
 * \brief get/set geninfo value
 * 
 * Set value of geninfo data.
 * 
 * \param raw  start of geninfo data
 * \param src  text data to assign
 * \param len  new text length
 * 
 * \return >= 0 on success
 */
extern int _mpt_geninfo_set(void *raw, const char *src, int len)
{
	struct metaInfo *info = raw;
	
	if (len < 0) {
		if (!src) {
			info->used = 0;
		}
		len = strlen(src);
	}
	if (len >= info->size) {
		return MPT_ERROR(MissingBuffer);
	}
	if (!src) {
		memset(info + 1, 0, len);
	} else {
		memcpy(info + 1, src, len);
	}
	((char *) (info + 1))[len] = 0;
	info->used = len + 1;
	
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
	
	return dlen;
}
/*!
 * \brief convert geninfo data
 * 
 * Get compatible data from geninfo.
 * 
 * \param raw  start of geninfo data
 * \param type property to change/request
 * \param dest data source to change property
 * 
 * \return >= 0 on success
 */
extern int _mpt_geninfo_conv(const void *raw, int type, void *ptr)
{
	const struct metaInfo *info = raw;
	void **dest = ptr;
	
	if (!type) {
		static const char types[] = { 's', 0 };
		if (dest) *dest = (void *) (info->used ? types : types + 1);
		return 0;
	}
	switch (type) {
	  case 's':
		if (dest) *dest = info->used ? (void *) (info + 1) : 0;
		return type;
	  case MPT_type_vector('c'):
		if (ptr) {
			struct iovec *vec = ptr;
			vec->iov_len = info->used;
			vec->iov_base = (void *) (info + 1);
		}
		return type;
	  default: return MPT_ERROR(BadType);
	}
	return type;
}
