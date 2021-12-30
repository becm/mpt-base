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
#include "types.h"

#include "values.h"

MPT_STRUCT(RawData) {
	MPT_INTERFACE(metatype) _mt;
	MPT_INTERFACE(rawdata)  _rd;
	
	MPT_STRUCT(refcount) _ref;
	
	_MPT_ARRAY_TYPE(rawdata_stage) st;
	long act, max;
};

/* convertable interface */
static int rd_conv(MPT_INTERFACE(convertable) *val, int type, void *ptr)
{
	MPT_STRUCT(RawData) *rd = MPT_baseaddr(RawData, val, _mt);
	int me = mpt_rawdata_typeid();
	
	if (me < 0) {
		me = MPT_ENUM(TypeMetaPtr);
	}
	else if (type == me) {
		if (ptr) *((const void **) ptr) = &rd->_rd;
		return me;
	}
	if (!type) {
		static const char fmt[] = { MPT_ENUM(TypeBufferPtr), 0 };
		if (ptr) *((const char **) ptr) = fmt;
		return me;
	}
	if (type == MPT_ENUM(TypeMetaPtr)) {
		if (ptr) *((const void **) ptr) = &rd->_mt;
		return me;
	}
	/* TODO: type info for stage data array
	if (type == MPT_ENUM(TypeArray)) {
		if (ptr) *((const void **) ptr) = &rd->st;
		return me;
	}*/
	return MPT_ERROR(BadType);
}
/* metatype interface */
static void rd_unref(MPT_INTERFACE(metatype) *ref)
{
	MPT_STRUCT(RawData) *rd = MPT_baseaddr(RawData, ref, _mt);
	
	if (mpt_refcount_lower(&rd->_ref)) {
		return;
	}
	mpt_array_clone(&rd->st, 0);
	free(rd);
}
static uintptr_t rd_ref(MPT_INTERFACE(metatype) *ref)
{
	MPT_STRUCT(RawData) *rd = MPT_baseaddr(RawData, ref, _mt);
	return mpt_refcount_raise(&rd->_ref);
}
static MPT_INTERFACE(metatype) *rd_clone(const MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
/* raw data interface */
static int rd_modify(MPT_INTERFACE(rawdata) *ptr, unsigned dim, int type, const void *src, size_t len, const MPT_STRUCT(valdest) *vd)
{
	const MPT_STRUCT(type_traits) *traits, *stage_traits;
	MPT_STRUCT(RawData) *rd = MPT_baseaddr(RawData, ptr, _rd);
	MPT_STRUCT(buffer) *buf;
	MPT_STRUCT(rawdata_stage) *st;
	MPT_STRUCT(value_store) *val;
	void *dest;
	uint32_t nc = 0, cycles;
	
	/* get traits for data type */
	if (!(traits = mpt_type_traits(type))) {
		return MPT_ERROR(BadType);
	}
	if (!(stage_traits = mpt_stage_traits())) {
		return MPT_ERROR(BadOperation);
	}
	/* determine target stage by cayle offset */
	if (!vd || !(nc = vd->cycle)) {
		nc = rd->act;
	}
	/* check target cycle limits */
	buf = rd->st._buf;
	cycles = buf ? buf->_used / stage_traits->size : 0;
	
	/* hard limit on cycle count */
	if (rd->max) {
		if (nc >= rd->max) {
			return MPT_ERROR(BadValue);
		}
	}
	/* soft restriction: no holes in cycles */
	else if (nc > cycles) {
		return MPT_ERROR(BadValue);
	}
	/* initialize buffer */
	if (!buf) {
		if (!(buf = _mpt_buffer_alloc((nc + 1) * stage_traits->size, 0))) {
			return MPT_ERROR(BadOperation);
		}
		buf->_content_traits = stage_traits;
		rd->st._buf = buf;
	}
	/* reserve stage data */
	if (!(st = mpt_array_slice(&rd->st, nc * stage_traits->size, stage_traits->size))) {
		return MPT_ERROR(BadOperation);
	}
	/* get value store for dimension */
	if (!(val = mpt_stage_data(st, dim))) {
		return MPT_ERROR(BadOperation);
	}
	/* assign new data */
	if (!(dest = mpt_array_set(&val->_d, traits, len, src, vd ? vd->offset : 0))) {
		return MPT_ERROR(BadOperation);
	}
	return val->_flags;
}
static int rd_advance(MPT_INTERFACE(rawdata) *ptr)
{
	MPT_STRUCT(RawData) *rd = MPT_baseaddr(RawData, ptr, _rd);
	const MPT_STRUCT(buffer) *buf;
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

static const MPT_STRUCT(value_store) *rd_values(const MPT_INTERFACE(rawdata) *ptr, unsigned dim, int nc)
{
	const MPT_STRUCT(RawData) *rd = MPT_baseaddr(RawData, ptr, _rd);
	const MPT_STRUCT(buffer) *buf;
	MPT_STRUCT(rawdata_stage) *st;
	
	/* query type of raw data */
	if (!(buf = rd->st._buf)) {
		return 0;
	}
	if (nc < 0) {
		nc = rd->act;
	}
	/* cycle does not exist */
	if (nc >= (int) (buf->_used / sizeof(*st))) {
		return 0;
	}
	/* get existing cycle dimension data */
	st = (void *) (buf + 1);
	return mpt_stage_data(st + nc, dim);
}

static int rd_dimensions(const MPT_INTERFACE(rawdata) *ptr, int part)
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
	return buf->_used / sizeof(MPT_STRUCT(value_store));
}

static int rd_stages(const MPT_INTERFACE(rawdata) *ptr)
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
	static const MPT_INTERFACE_VPTR(metatype) meta = {
		{ rd_conv },
		rd_unref,
		rd_ref,
		rd_clone
	};
	static const MPT_INTERFACE_VPTR(rawdata) raw = {
		rd_modify,
		rd_advance,
		rd_values,
		rd_dimensions,
		rd_stages
	};
	static const MPT_STRUCT(RawData) def = {
		{ &meta },
		{ &raw },
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
