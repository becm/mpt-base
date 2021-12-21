/*
 * MPT C++ cycle implementation
 */

#include <cerrno>
#include <limits>

#include <sys/uio.h>

#include "types.h"
#include "message.h"

#include "values.h"

__MPT_NAMESPACE_BEGIN

template class reference<cycle>;
template class reference_array<cycle>;

template class type_properties<reference<cycle> >;
template class type_properties<cycle::stage>;

value_store *rawdata_stage::values(long dim)
{
	if (dim < 0) {
		dim += values().size();
		if (dim < 0) {
			errno = EINVAL;
			return 0;
		}
	}
	return mpt_stage_data(this, dim);
}

cycle::cycle() : _act(0), _max_dimensions(0), _flags(0)
{ }
cycle::~cycle()
{ }
void cycle::unref()
{
	delete this;
}
cycle *cycle::clone() const
{
	cycle *ret = new cycle();
	
	ret->_stages = _stages;
	ret->_act = _act;
	ret->_max_dimensions = _max_dimensions;
	ret->_flags = _flags;
	
	return ret;
}

int cycle::modify(unsigned dim, int type, const void *src, size_t len, const valdest *vd)
{
	if (_max_dimensions && dim >= _max_dimensions) {
		return BadArgument;
	}
	const struct type_traits *source_traits;
	if (!(source_traits = type_traits::get(type))) {
		return BadType;
	}
	if (len % source_traits->size) {
		return MissingData;
	}
	len /= source_traits->size;
	if ((SIZE_MAX / sizeof(double)) < len) {
		return BadArgument;
	}
	int nc;
	if (!vd || !(nc = vd->cycle)) {
		nc = _act;
	} else {
		--nc;
	}
	stage *st;
	if (!(st = _stages.get(nc))) {
		if (_flags & LimitStages) {
			return BadValue;
		}
		if (!(st = _stages.unique_array<stage>::insert(nc))) {
			return BadOperation;
		}
	}
	value_store *val = st->rawdata_stage::values(dim);
	if (!val) {
		return BadValue;
	}
	long off = vd ? vd->offset : 0;
	double *ptr;
	if (!(ptr = static_cast<double *>(val->reserve(len + off, *type_properties<double>::traits())))) {
		return BadOperation;
	}
	if (!src) {
		memset(ptr, 0, len * sizeof(double));
	}
	
	else if (source_traits == type_properties<long double>::traits()) {
		copy(len, static_cast<const long double *>(src), ptr);
	}
	else if (source_traits == type_properties<double>::traits()) {
		memcpy(ptr, src, len * sizeof(double));
	}
	else if (source_traits == type_properties<float>::traits()) {
		copy(len, static_cast<const float *>(src), ptr);
	}
	
	else if (source_traits == type_properties<int8_t>::traits()) {
		copy(len, static_cast<const int8_t  *>(src), ptr);
	}
	else if (source_traits == type_properties<int16_t>::traits()) {
		copy(len, static_cast<const int16_t *>(src), ptr);
	}
	else if (source_traits == type_properties<int32_t>::traits()) {
		copy(len, static_cast<const int32_t *>(src), ptr);
	}
	else if (source_traits == type_properties<int64_t>::traits()) {
		copy(len, static_cast<const int64_t *>(src), ptr);
	}
	
	else if (source_traits == type_properties<uint8_t>::traits()) {
		copy(len, static_cast<const uint8_t  *>(src), ptr);
	}
	else if (source_traits == type_properties<uint16_t>::traits()) {
		copy(len, static_cast<const uint16_t *>(src), ptr);
	}
	else if (source_traits == type_properties<uint32_t>::traits()) {
		copy(len, static_cast<const uint32_t *>(src), ptr);
	}
	else if (source_traits == type_properties<uint64_t>::traits()) {
		copy(len, static_cast<const uint64_t *>(src), ptr);
	}
	
	else {
		return BadType;
	}
	// update dimension and invalidate view
	val->set_modified();
	st->invalidate();
	return val->flags();
}
const MPT_STRUCT(value_store) *cycle::values(unsigned dim, int nc) const
{
	if (_max_dimensions && dim >= _max_dimensions) {
		errno = EINVAL;
		return 0;
	}
	if (nc < 0) {
		nc = _act;
	}
	stage *st;
	if (!(st = _stages.get(nc))) {
		return 0;
	}
	return st->rawdata_stage::values(dim);
}
int cycle::advance()
{
	int n = _act + 1;
	rawdata_stage *st;
	if ((st = _stages.get(n))) {
		_act = n;
		return n;
	}
	if (_flags & LimitStages) {
		_act = 0;
		return _stages.length() ? 0 : BadValue;
	}
	// no advance if current stage not assigned
	if (!(st = _stages.get(_act))) {
		return BadArgument;
	}
	// no advance if current stage empty
	long dim = st->dimension_count();
	if (dim <= 0) {
		return _act;
	}
	_act = n;
	return n;
}
long cycle::dimension_count(int n) const
{
	if (n < 0) {
		n = _act;
	}
	stage *st = _stages.get(n);
	return st ? st->dimension_count() : (long) BadArgument;
}
long cycle::stage_count() const
{
	return _stages.length();
}
bool cycle::limit_stages(size_t max)
{
	if (!max) {
		_flags &= LimitStages;
		return true;
	}
	size_t curr = _stages.length();
	if (max < curr) {
		return false;
	}
	if (curr < max && !_stages.unique_array<stage>::insert(max - 1)) {
		return false;
	}
	_flags |= LimitStages;
	return true;
}
void cycle::limit_dimensions(uint8_t md)
{
	_max_dimensions = md;
}

bool cycle::stage::transform(const class transform &tr)
{
	return _values.set(tr, rawdata_stage::values());
}
__MPT_NAMESPACE_END
