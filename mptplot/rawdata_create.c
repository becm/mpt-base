/*!
 * modify world cycle structure.
 */

#include <string.h>
#include <stdlib.h>

#include <errno.h>
#include <limits.h>

#include <sys/uio.h>

#include "meta.h"
#include "message.h"

#include "values.h"

MPT_STRUCT(RawData) {
	MPT_INTERFACE(metatype) _mt;
	MPT_INTERFACE(rawdata)  _rd;
	
	MPT_STRUCT(refcount) _ref;
	
	_MPT_ARRAY_TYPE(rawdata_stage) st;
	long act, max;
};
 
/* reference interface */
static void rdUnref(MPT_INTERFACE(reference) *ref)
{
	MPT_STRUCT(RawData) *rd = MPT_baseaddr(RawData, ref, _mt);
	MPT_STRUCT(buffer) *buf;
	
	if (mpt_refcount_lower(&rd->_ref)) {
		return;
	}
	if ((buf = rd->st._buf)) {
		MPT_STRUCT(rawdata_stage) *st = (void *) (buf + 1);
		size_t i, len = buf->_used/sizeof(*st);
		
		for (i = 0; i < len; ++i) {
			mpt_stage_fini(st + i);
		}
		mpt_array_clone(&rd->st, 0);
	}
	rd->_mt._vptr = 0;
	rd->_rd._vptr = 0;
	free(rd);
}
static uintptr_t rdRef(MPT_INTERFACE(reference) *ref)
{
	MPT_STRUCT(RawData) *rd = MPT_baseaddr(RawData, ref, _mt);
	return mpt_refcount_raise(&rd->_ref);
}
/* metatype interface */
static int rdConv(const MPT_INTERFACE(metatype) *mt, int type, void *ptr)
{
	MPT_STRUCT(RawData) *rd = MPT_baseaddr(RawData, mt, _mt);
	int me = mpt_rawdata_typeid();
	
	if (me < 0) {
		me = MPT_ENUM(TypeMeta);
	}
	if (!type) {
		static const char fmt[] = { MPT_ENUM(TypeMeta), MPT_ENUM(TypeArray), 0 };
		if (ptr) *((const char **) ptr) = fmt;
		return me;
	}
	if (type == MPT_ENUM(TypeMeta)) {
		if (ptr) *((const void **) ptr) = &rd->_mt;
		return me;
	}
	if (type == me) {
		if (ptr) *((const void **) ptr) = &rd->_rd;
		return MPT_ENUM(TypeMeta);
	}
	if (type == MPT_ENUM(TypeArray)) {
		if (ptr) *((const void **) ptr) = &rd->st;
		return me;
	}
	return MPT_ERROR(BadType);
}
static MPT_INTERFACE(metatype) *rdClone(const MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
/* raw data interface */
static int rdModify(MPT_INTERFACE(rawdata) *ptr, unsigned dim, int fmt, const void *src, size_t len, const MPT_STRUCT(valdest) *vd)
{
	MPT_STRUCT(RawData) *rd = MPT_baseaddr(RawData, ptr, _rd);
	MPT_STRUCT(buffer) *buf;
	MPT_STRUCT(rawdata_stage) *st;
	MPT_STRUCT(typed_array) *arr;
	long nc;
	
	if (!vd || !(nc = vd->cycle)) {
		nc = rd->act;
	}
	if (!(buf = rd->st._buf)
	    || nc >= (long) (buf->_used / sizeof(*st))) {
		if (!rd->max || nc >= rd->max) {
			return MPT_ERROR(BadValue);
		}
		if (!(st = mpt_array_slice(&rd->st, nc * sizeof(*st), sizeof(*st)))) {
			return MPT_ERROR(BadOperation);
		}
		memset(st, 0, sizeof(*st));
	}
	else {
		st = (void *) (buf + 1);
		st += nc;
	}
	if (!(arr = mpt_stage_data(st, dim))) {
		return MPT_ERROR(BadOperation);
	}
	if (!arr->_type) {
		int size;
		if (fmt > 0xff || (size = mpt_valsize(fmt)) <= 0) {
			return MPT_ERROR(BadType);
		}
		if (size > 0xff) {
			return MPT_ERROR(BadType);
		}
		arr->_type = fmt;
		arr->_esize = size;
		
		fmt = mpt_msgvalfmt_code(fmt);
		arr->_code = fmt < 0 ? 0 : fmt;
	}
	else if ((uint32_t) fmt != arr->_type) {
		return MPT_ERROR(BadType);
	}
	if (len) {
		void *dest;
		size_t pos = 0;
		
		if (len % arr->_esize) {
			return MPT_ERROR(BadValue);
		}
		if (vd) {
			pos = vd->offset * arr->_esize;
		}
		if (!(dest = mpt_array_slice(&arr->_d, pos, len))) {
			return MPT_ERROR(BadOperation);
		}
		if (src) {
			memcpy(dest, src, len);
		} else {
			memset(dest, 0, len);
		}
		arr->_flags |= 0x1;
	}
	return arr->_flags;
}
static int rdAdvance(MPT_INTERFACE(rawdata) *ptr)
{
	MPT_STRUCT(RawData) *rd = MPT_baseaddr(RawData, ptr, _rd);
	MPT_STRUCT(buffer) *buf;
	long act;
	
	/* limit cycle size */
	if ((act = rd->act + 1) >= rd->max) {
		act = 0;
	}
	/* reuse existing cycle */
	if ((buf = rd->st._buf)
	    && act < (long) (buf->_used / sizeof(MPT_STRUCT(rawdata_stage)))) {
		return act;
	}
	/* add cycle placeholder */
	if (!mpt_array_append(&rd->st, sizeof(MPT_STRUCT(rawdata_stage)), 0)) {
		return 0;
	}
	rd->act = act;
	return act;
}

static int rdValues(const MPT_INTERFACE(rawdata) *ptr, unsigned dim, struct iovec *vec, int nc)
{
	const MPT_STRUCT(RawData) *rd = MPT_baseaddr(RawData, ptr, _rd);
	const MPT_STRUCT(buffer) *buf;
	MPT_STRUCT(rawdata_stage) *st;
	const MPT_STRUCT(typed_array) *arr;
	
	/* query type of raw data */
	if (!(buf = rd->st._buf)) {
		return 0;
	}
	if (nc < 0) {
		nc = rd->act;
	}
	/* cycle does not exist */
	if (nc >= (int) (buf->_used / sizeof(*st))) {
		return MPT_ERROR(BadValue);
	}
	/* get existing cycle dimension data */
	st = (void *) (buf + 1);
	if (!(arr = mpt_stage_data(st + nc, dim))) {
		return MPT_ERROR(BadValue);
	}
	if (!arr->_type) {
		return MPT_ERROR(BadOperation);
	}
	if (vec) {
		if ((buf = arr->_d._buf)) {
			vec->iov_base = (void *) (buf + 1);
			vec->iov_len  = buf->_used;
		} else {
			vec->iov_base = 0;
			vec->iov_len  = 0;
		}
	}
	return arr->_type;
}

static int rdDimensions(const MPT_INTERFACE(rawdata) *ptr, int part)
{
	const MPT_STRUCT(RawData) *rd = MPT_baseaddr(RawData, ptr, _rd);
	const MPT_STRUCT(buffer) *buf;
	const MPT_STRUCT(rawdata_stage) *st;
	int max;
	
	if (!(buf = rd->st._buf)) {
		return MPT_ERROR(BadOperation);
	}
	st = (void *) (buf + 1);
	max = buf->_used / sizeof(*st);
	
	if (part < 0) {
		part = rd->act;
	}
	if (part >= max) {
		return MPT_ERROR(BadValue);
	}
	if (!(buf = st[part]._d._buf)) {
		return 0;
	}
	return buf->_used / sizeof(MPT_STRUCT(typed_array));
}

static int rdStages(const MPT_INTERFACE(rawdata) *ptr)
{
	const MPT_STRUCT(RawData) *rd = MPT_baseaddr(RawData, ptr, _rd);
	const MPT_STRUCT(buffer) *buf;
	
	if (!(buf = rd->st._buf)) {
		return 0;
	}
	return buf->_used / sizeof(MPT_STRUCT(rawdata_stage));
}

/*!
 * \ingroup mptPlot
 * \brief raw data store
 * 
 * Create raw data storage with cycle instances
 * 
 * \param max limit cycle count
 * 
 * \return new raw data buffer instance
 */
extern MPT_INTERFACE(metatype) *mpt_rawdata_create(long max)
{
	static const MPT_INTERFACE_VPTR(metatype) rawMeta = {
		{ rdUnref, rdRef },
		rdConv,
		rdClone
	};
	static const MPT_INTERFACE_VPTR(rawdata) rawData = {
		rdModify,
		rdAdvance,
		rdValues,
		rdDimensions,
		rdStages
	};
	static const MPT_STRUCT(RawData) def = {
		{ &rawMeta },
		{ &rawData },
		{ 1 },
		MPT_ARRAY_INIT,
		0, 0
	};
	MPT_STRUCT(RawData) *rd;
	
	if (max >= LONG_MAX) {
		errno = EINVAL;
		return 0;
	}
	if (max < 0) {
		max = 0;
	}
	if (!(rd = malloc(sizeof(*rd)))) {
		return 0;
	}
	*rd = def;
	rd->max = max;
	
	return &rd->_mt;
}
