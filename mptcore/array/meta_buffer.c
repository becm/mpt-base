/*!
 * \file
 * control handler for buffer storage.
 */

#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

#include <sys/uio.h>

#include "array.h"

MPT_STRUCT(metaBuffer) {
	MPT_STRUCT(array)        arr;
	MPT_INTERFACE(metatype) _meta;
	uint64_t                _info;
};

static int bufferUnref(MPT_INTERFACE(metatype) *meta)
{
	MPT_STRUCT(metaBuffer) *mb = MPT_reladdr(metaBuffer, meta, _meta, arr);
	int c = _mpt_geninfo_unref(&mb->_info);
	if (c) return c;
	mpt_array_clone(&mb->arr, 0);
	free(mb);
	return 0;
}
static MPT_INTERFACE(metatype) *bufferAddref(MPT_INTERFACE(metatype) *meta)
{
	MPT_STRUCT(metaBuffer) *mb = MPT_reladdr(metaBuffer, meta, _meta, arr);
	return (_mpt_geninfo_addref(&mb->_info)) ? meta : 0;
}

static int setArray(MPT_STRUCT(array) *arr, MPT_INTERFACE(source) *src)
{
	struct iovec tmp;
	void *base, *data;
	int len;
	
	if ((len = src->_vptr->conv(src, MPT_ENUM(TypeVector), &tmp)) >= 0) {
		data = tmp.iov_base;
		len  = tmp.iov_len;
	}
	else if ((len = src->_vptr->conv(src, 's', &data)) >= 0) {
		if (!len && data) len = strlen(data);
		if (len) ++len;
	}
	else {
		errno = ENOTSUP;
		return -1;
	}
	
	if (!(base = mpt_array_slice(arr, 0, len))) {
		return len ? -1 : 0;
	}
	memcpy(base, data, len);
	
	return arr->_buf->used = len;
}



static int bufferProperty(MPT_INTERFACE(metatype) *meta, MPT_STRUCT(property) *prop, MPT_INTERFACE(source) *src)
{
	static const uintptr_t _size = 0;
	MPT_STRUCT(metaBuffer) *mb = MPT_reladdr(metaBuffer, meta, _meta, arr);
	int ret;
	
	if (!prop) {
		return src ? setArray(&mb->arr, src) : MPT_ENUM(TypeArray);
	}
	if (!prop->name) {
		return -3;
	}
	else if (*prop->name) {
		if (strcmp("used", prop->name)) {
			return -3;
		}
		prop->desc = "buffer metatype";
		prop->val.fmt = "L";
		prop->val.ptr = mb->arr._buf ? &mb->arr._buf->used : &_size;
		return 0;
	}
	else {
		static const char fmt[2] = { MPT_ENUM(TypeArray) };
		ret = 0;
		if (src && (ret = setArray(&mb->arr, src)) < 0) {
			return ret;
		}
		prop->val.fmt = fmt;
		prop->val.ptr = &mb->arr;
	}
	
	prop->name = "array";
	prop->desc = "generic resizable data array";
	
	return ret;
}

static void *bufferCast(MPT_INTERFACE(metatype) *meta, int type)
{
	MPT_STRUCT(metaBuffer) *mb = MPT_reladdr(metaBuffer, meta, _meta, arr);
	
	switch (type) {
	  case MPT_ENUM(TypeMeta): return meta;
	  case MPT_ENUM(TypeArray): return &mb->arr;
	  case 's': return mpt_array_string(&mb->arr);
	  default: return 0;
	}
}

static const MPT_INTERFACE_VPTR(metatype) _vptr_buffer = {
	bufferUnref,
	bufferAddref,
	bufferProperty,
	bufferCast
};

/*!
 * \ingroup mptArray
 * \brief create buffer metatype
 * 
 * Create metatype with dynamic data size.
 * 
 * \param name	name of created metatype
 * \param len	initial data size
 * \param data	initial buffer data
 * 
 * \return new metatype
 */
extern MPT_INTERFACE(metatype) *mpt_meta_buffer(size_t len, const void *data)
{
	MPT_STRUCT(metaBuffer) *mb;
	
	if (!(mb = malloc(sizeof(*mb)))) {
		return 0;
	}
	if (_mpt_geninfo_init(&mb->_info, sizeof(mb->_info), 1) < 0) {
		free(mb);
		return 0;
	}
	
	mb->arr._buf = 0;
	mb->_meta._vptr = &_vptr_buffer;
	
	if (!len || mpt_array_append(&mb->arr, len, data)) return &mb->_meta;
	
	free(mb);
	
	return 0;
}

