/*
 * mpt C++ library
 *   type ID classification
 */

#include <limits>

#include "array.h"

#include "convert.h"

__MPT_NAMESPACE_BEGIN

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
float80::operator long double() const
{
	long double v = 0;
	mpt_float80_decode(1, this, &v);
	return v;
}

value::value(const value &val) : _addr(val._addr), _type(val._type), _namespace(val._namespace)
{ }

value &value::operator=(const value &val)
{
	_addr = val._addr;
	_type = val._type;
	_namespace = val._namespace;
	return *this;
}

int value::convert(int type, void *ptr) const
{
	return mpt_value_convert(this, type, ptr);
}

bool value::set(int t, const void *d, int ns)
{
	if (t < 0 || t > std::numeric_limits<decltype(_type)>::max()) {
		return false;
	}
	if (ns < 0 || ns > std::numeric_limits<decltype(_namespace)>::max()) {
		return false;
	}
	_addr = d;
	_type = t;
	_namespace = ns;
	return true;
}
const char *value::string() const
{
	if (_type == 's') {
		const char *str;
		return _addr && (str = *static_cast<char * const *>(_addr)) ? str : "";
	}
	if (!_type || !_addr) {
		return 0;
	}
	const void *ptr = _addr;
	return mpt_data_tostring(&ptr, _type, 0);
}
const struct iovec *value::vector(int to) const
{
	if (_namespace || !_type || !_addr) {
		return 0;
	}
	const struct iovec *vec = reinterpret_cast<const struct iovec *>(_addr);
	if (!vec) {
		return 0;
	}
	// accept all vector types
	if (to < 0) {
		return MPT_type_isVector(_type) ? vec : 0;
	}
	// raw data only
	if (to == 0) {
		return (_type == TypeVector) ? vec : 0;
	}
	// content must match specific type
	return (_type == MPT_type_toVector(to)) ? vec : 0;
}
const array *value::array(int to) const
{
	if (_namespace || !_type || !_addr) {
		return 0;
	}
	// invalid source type
	if (_type != MPT_ENUM(TypeArray)) {
		return 0;
	}
	const mpt::array *arr = reinterpret_cast<const mpt::array *>(_addr);
	if (to < 0 || !arr) {
		return arr;
	}
	const array::content *d = arr->data();
	const struct type_traits *traits = d ? d->content_traits() : 0;
	// raw data may not have type traits
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
