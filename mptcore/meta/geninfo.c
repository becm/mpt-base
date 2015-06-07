/*!
 * generic metatype info
 */

#include <errno.h>
#include <string.h>

#include <sys/uio.h>

#include "core.h"

struct metaInfo {
	uint8_t  size, /* free allocated memory */
	         used; /* used data size */
	uint16_t line;
	uint32_t ref;
};

/*!
 * \brief increase geninfo reference count
 * 
 * increase reference count of geninfo data
 * 
 * \param raw start of geninfo data
 * 
 * \return new reference count
 */
extern uint32_t _mpt_geninfo_addref(uint64_t *raw)
{
	struct metaInfo *info = (void *) raw;
	int32_t c = info->ref;
	
	if (!c) { errno = EINVAL; return 0; }
	
	if ((c = (++info->ref)) > 0) return c;
	
	--info->ref;
	errno = ERANGE;
	
	return 0;
}
/*!
 * \brief decrease geninfo reference count
 * 
 * decrease reference count of geninfo data
 * 
 * \param raw start of geninfo data
 * 
 * \return new reference count
 */
extern uint32_t _mpt_geninfo_unref(uint64_t *raw)
{
	struct metaInfo *info = (void *) raw;
	uint32_t c = info->ref;
	
	if (!c) return 1;
	return --info->ref;
}
static int geninfoSet(struct metaInfo *info, MPT_INTERFACE(source) *src)
{
	struct iovec vec;
	int len;
	
	if ((len = src->_vptr->conv(src, 's', &vec.iov_base)) >= 0) {
		if (!(vec.iov_len = len)) {
			vec.iov_len = vec.iov_base ? strlen(vec.iov_base) : 0;
		}
	}
	else if ((len = src->_vptr->conv(src, MPT_ENUM(TypeVector) | 'c', &vec)) < 0) {
		return -1;
	}
	if (!len || !vec.iov_len) {
		info->used = 0;
		return len;
	}
	if (vec.iov_len >= info->size) {
		return -2;
	}
	if (!vec.iov_base) {
		return -3;
	}
	vec.iov_base = memcpy(info+1, vec.iov_base, vec.iov_len);
	((char *) vec.iov_base)[vec.iov_len] = 0;
	
	info->used = vec.iov_len + 1;
	
	return len;
}
/*!
 * \brief get/set geninfo property
 * 
 * Set property of geninfo data.
 * Return 0 for type request!
 * 
 * \param raw  start of geninfo data
 * \param prop property to change/request
 * \param src  data source to change property
 * 
 * \return >= 0 on success
 */
extern int _mpt_geninfo_property(uint64_t *raw, MPT_STRUCT(property) *prop, MPT_INTERFACE(source) *src)
{
	static const MPT_STRUCT(property) metaProp[] = {
		{ "store", "default storage element", { "", 0 } },
		{ "line",  "origin line", { "H", 0 } },
		{ "used",  "current data usage", { "B", 0 } },
		{ "size",  "max. available size", { "B", 0 } }
	};
	struct metaInfo *info = (void *) raw;
	
	if (!prop) {
		return src ? geninfoSet(info, src) : info->used;
	}
	if (!prop->name) {
		return src ? -1 : -3;
	}
	
	if (!*prop->name) {
		int ret = info->used;
		
		if (src && (ret = geninfoSet(info, src)) < 0) return -2;
		
		prop->name = metaProp[0].name;
		prop->desc = metaProp[0].desc;
		prop->val.fmt = 0;
		prop->val.ptr = info->used ? info + 1 : 0;
		return ret;
	}
	if (!(strcasecmp(prop->name, metaProp[1].name))) {
		int ret = info->used;
		if (src && (ret = src->_vptr->conv(src, 'H', &info->line)) < 0) {
			return ret;
		}
		*prop = metaProp[1];
		prop->val.ptr = &info->line;
		return ret;
	}
	if (src) {
		return -1;
	}
	if (!strcasecmp(prop->name, metaProp[2].name)) {
		*prop = metaProp[2];
		prop->val.ptr = &info->used;
		return 1;
	}
	if (!strcasecmp(prop->name, metaProp[3].name)) {
		*prop = metaProp[3];
		prop->val.ptr = &info->size;
		return 1;
	}
	errno = ENOTSUP;
	return -1;
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
extern int _mpt_geninfo_init(void *raw, size_t dlen, uint32_t ref)
{
	struct metaInfo *info = raw;
	
	if (dlen < sizeof(*info)) {
		return -2;
	}
	dlen -= sizeof(*info);
	if (dlen > UINT8_MAX) {
		dlen = UINT8_MAX;
	}
	info->size = dlen;
	info->used = 0;
	info->line = 0;
	info->ref  = ref;
	
	return dlen;
}
