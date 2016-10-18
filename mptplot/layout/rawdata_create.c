/*!
 * modify world cycle structure.
 */

#include <string.h>
#include <stdlib.h>

#include <sys/uio.h>

#include "array.h"
#include "layout.h"

struct RawData
{
	MPT_INTERFACE(rawdata) gen;
	_MPT_ARRAY_TYPE(cycle) cyc; /* array of arrays */
	size_t act, max;
};

static void rdUnref(MPT_INTERFACE(unrefable) *ptr)
{
	struct RawData *rd = (void *) ptr;
	MPT_STRUCT(buffer) *buf;
	
	if ((buf = rd->cyc._buf)) {
		MPT_STRUCT(cycle) *cyc = (void *) (buf+1);
		size_t i, len = buf->used/sizeof(*cyc);
		
		for (i = 0; i < len; ++i) {
			mpt_cycle_fini(cyc + i);
		}
		if (buf->resize) {
			buf->resize(buf, 0);
		}
		rd->cyc._buf = 0;
	}
	rd->gen._vptr = 0;
	free(rd);
}
static int rdModify(MPT_INTERFACE(rawdata) *ptr, unsigned dim, int fmt, const void *src, size_t pos, size_t len, int nc)
{
	struct RawData *rd = (void *) ptr;
	MPT_STRUCT(typed_array) *arr;
	MPT_STRUCT(buffer) *buf;
	MPT_STRUCT(cycle) *cyc;
	
	if (nc < 0) {
		nc = rd->act;
	}
	if (!(buf = rd->cyc._buf)
	    || nc >= (int) (buf->used / sizeof(*cyc))) {
		if (!rd->max || nc >= (int) rd->max) {
			return MPT_ERROR(BadValue);
		}
		if (!(fmt & MPT_ENUM(ValueCreate))) {
			return MPT_ERROR(BadOperation);
		}
		if (!(cyc = mpt_array_slice(&rd->cyc, nc * sizeof(*cyc), sizeof(*cyc)))) {
			return MPT_ERROR(BadOperation);
		}
		memset(cyc, 0, sizeof(*cyc));
	}
	else {
		cyc = (void *) (buf+1);
		cyc += nc;
	}
	if (!(arr = mpt_cycle_data(cyc, dim, fmt))) {
		return MPT_ERROR(BadOperation);
	}
	if (len) {
		void *dest;
		if (!(dest = mpt_array_slice(&arr->_d, pos, len))) {
			return MPT_ERROR(BadOperation);
		}
		if (src) {
			memcpy(dest, src, len);
		} else {
			memset(dest, 0, len);
		}
		arr->_flags |= MPT_ENUM(ValueChange);
	}
	return arr->_format + (arr->_flags & ~0xff);
}
static int rdAdvance(MPT_INTERFACE(rawdata) *ptr)
{
	struct RawData *rd = (void *) ptr;
	MPT_STRUCT(buffer) *buf;
	size_t act;
	
	/* limit cycle size */
	if ((act = rd->act + 1) >= rd->max) {
		act = 0;
	}
	/* reuse existing cycle */
	if ((buf = rd->cyc._buf)
	    && act < buf->used / sizeof(MPT_STRUCT(cycle))) {
		return act;
	}
	/* add cycle placeholder */
	if (!mpt_array_append(&rd->cyc, sizeof(MPT_STRUCT(cycle)), 0)) {
		return 0;
	}
	rd->act = act;
	return act;
}

static int rdValues(const MPT_INTERFACE(rawdata) *ptr, unsigned dim, struct iovec *vec, int nc)
{
	const struct RawData *rd = (void *) ptr;
	const MPT_STRUCT(buffer) *buf;
	MPT_STRUCT(cycle) *cyc;
	const MPT_STRUCT(typed_array) *arr;
	
	/* query type of raw data */
	if (!(buf = rd->cyc._buf)) {
		return 0;
	}
	if (nc < 0) {
		nc = rd->act;
	}
	/* cycle does not exist */
	if (nc >= (int) (buf->used/sizeof(*cyc))) {
		return MPT_ERROR(BadValue);
	}
	/* get existing cycle dimension data */
	cyc = (void *) (buf+1);
	if (!(arr = mpt_cycle_data(cyc + nc, dim, -1))) {
		return MPT_ERROR(BadValue);
	}
	if (!arr->_format) {
		return MPT_ERROR(BadOperation);
	}
	if (vec) {
		if ((buf = arr->_d._buf)) {
			vec->iov_base = (void *) (buf + 1);
			vec->iov_len  = buf->used;
		} else {
			vec->iov_base = 0;
			vec->iov_len  = 0;
		}
	}
	return arr->_format + arr->_flags * 0x100;
}

static int rdDimensions(const MPT_INTERFACE(rawdata) *ptr, int part)
{
	const struct RawData *rd = (void *) ptr;
	const MPT_STRUCT(buffer) *buf;
	const MPT_STRUCT(cycle) *cyc;
	int max;
	
	if (!(buf = rd->cyc._buf)) {
		return MPT_ERROR(BadOperation);
	}
	cyc = (void *) (buf + 1);
	max = buf->used/sizeof(*cyc);
	
	if (part < 0) {
		part = rd->act;
	}
	if (part >= max) {
		return MPT_ERROR(BadValue);
	}
	if (!(buf = cyc[part]._d._buf)) {
		return 0;
	}
	return buf->used/sizeof(MPT_STRUCT(typed_array));
}

static int rdCycles(const MPT_INTERFACE(rawdata) *ptr)
{
	const struct RawData *rd = (void *) ptr;
	const MPT_STRUCT(buffer) *buf;
	
	if (!(buf = rd->cyc._buf)) {
		return 0;
	}
	return buf->used/sizeof(MPT_STRUCT(cycle));
}

static const MPT_INTERFACE_VPTR(rawdata) _vptr = {
	{ rdUnref },
	rdModify, rdAdvance,
	rdValues, rdDimensions, rdCycles
};

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
extern MPT_INTERFACE(rawdata) *mpt_cycle_create(size_t max)
{
	struct RawData *rd;
	
	if (!(rd = malloc(sizeof(*rd)))) {
		return 0;
	}
	rd->gen._vptr = &_vptr;
	rd->cyc._buf = 0;
	rd->act = 0;
	rd->max = max;
	
	return &rd->gen;
}
