/*
 * mpt C++ library
 *   type ID classification
 */

#include <sys/uio.h>

#include "array.h"
#include "types.h"

#include "convert.h"

__MPT_NAMESPACE_BEGIN

// conversion wrapper
int convert(const void **val, int fmt, void *dest, int type)
{
	return mpt_data_convert(val, fmt, dest, type);
}

// byte order adaption
float swapOrder(float v)
{
	mpt_bswap_32(1, reinterpret_cast<uint32_t *>(&v));
	return v;
}
double swapOrder(double v)
{
	mpt_bswap_64(1, reinterpret_cast<uint64_t *>(&v));
	return v;
}
float80 swapOrder(float80 v)
{
	mpt_bswap_80(1, &v);
	return v;
}

// transport value operations
float80 &float80::operator= (long double val)
{
	mpt_float80_encode(1, &val, this);
	return *this;
}
long double float80::value() const
{
	long double v = 0;
	mpt_float80_decode(1, this, &v);
	return v;
}

value::value(const char *val) : domain(0), type(0), _bufsize(sizeof(_buf)), _pad(0), ptr(0)
{
	set(val);
}

value::value(const value &val) : domain(0), type(0), _bufsize(sizeof(_buf)), _pad(0), ptr(0)
{
	*this = val;
}

value &value::operator=(const value &val)
{
	domain = val.domain;
	type = val.type;
	if (val.ptr != val._buf || val._bufsize > _bufsize) {
		ptr = val.ptr;
	}
	else {
		ptr = memcpy(_buf, val._buf, val._bufsize);
	}
	return *this;
}

bool value::set(const char *val)
{
	if (val) {
		int len = strlen(val) + 1;
		if (_bufsize > (sizeof(val) + len)) {
			val = static_cast<char *>(memcpy(_buf + sizeof(val), val, len));
		}
	}
	if (_bufsize < sizeof(val)) {
		return false;
	}
	domain = 0;
	type = 's';
	ptr = memcpy(_buf, &val, sizeof(val));
	return true;
}

bool value::set(int t, const void *d)
{
	const type_traits *traits = type_traits::get(type);
	
	if (traits && !traits->fini && !traits->init && traits->size && (traits->size < _bufsize)) {
		if (d) {
			ptr = memcpy(_buf, d, traits->size);
		} else {
			ptr = memset(_buf, 0, traits->size);
		}
	}
	else {
		ptr = d;
	}
	domain = 0;
	type = t;
	return true;
}
const char *value::string() const
{
	if (type == 's') {
		const char *str = *static_cast<char * const *>(ptr);
		return str ? str : "";
	}
	if (!type) {
		return 0;
	}
	const void *str = ptr;
	return mpt_data_tostring(&str, type, 0);
}
const struct iovec *value::vector(int to) const
{
	if (domain || !type || !ptr) {
		return 0;
	}
	const struct iovec *vec = reinterpret_cast<const struct iovec *>(ptr);
	if (!vec) {
		return 0;
	}
	// accept all vector types
	if (to < 0) {
		return MPT_type_isVector(type) ? vec : 0;
	}
	// raw type only
	if (to == 0) {
		return (type == TypeVector) ? vec : 0;
	}
	// content must match specific type
	return (type == MPT_type_toVector(to)) ? vec : 0;
}
const array *value::array(int to) const
{
	if (domain || !type || !ptr) {
		return 0;
	}
	// invalid source type
	if (type != MPT_ENUM(TypeArray)) {
		return 0;
	}
	const mpt::array *arr = reinterpret_cast<const mpt::array *>(ptr);
	if (to < 0 || !arr) {
		return arr;
	}
	const array::content *d = arr->data();
	const struct type_traits *traits = d ? d->content_traits() : 0;
	// raw type may not have data traits
	if (!to) {
		return traits ? 0 : arr;
	}
	// data traits must match requested type
	if (traits && (type_traits::get(to) == traits)) {
		return arr;
	}
	return 0;
}

__MPT_NAMESPACE_END
