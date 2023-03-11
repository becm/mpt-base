/*!
 * MPT C++ library
 *   interfaces to output stream
 */

#include "object.h"

#include "convert.h"

static ssize_t writeOutStream(void *p, const char *str, size_t len)
{
	static_cast<std::ostream *>(p)->write(str, len);
	return len;
}

std::ostream &operator<<(std::ostream &o, const mpt::value &val)
{
	// skip output for unset/empty value
	if (val.type() == 0 || !val.data()) {
		return o;
	}
	// print value content, invalidate stream on unsupported data
	if (mpt_print_value(&val, writeOutStream, &o) < 0) {
		o.setstate(std::ios::badbit | std::ios::failbit);
	}
	return o;
}
std::ostream &operator<<(std::ostream &o, const mpt::object &obj)
{
	if (mpt_print_object(&obj, writeOutStream, &o) < 0) {
		o.setstate(std::ios::badbit | std::ios::failbit);
	}
	return o;
}
std::ostream &operator<<(std::ostream &o, mpt::convertable &conv)
{
	if (mpt_print_convertable(&conv, writeOutStream, &o) < 0) {
		o.setstate(std::ios::badbit | std::ios::failbit);
	}
	return o;
}
template <> std::ostream &operator<< <char>(std::ostream &o, const mpt::span<char> &p)
{
	o.write(p.begin(), p.size());
	return o;
}
template <> std::ostream &operator<< <const char>(std::ostream &o, const mpt::span<const char> &p)
{
	o.write(p.begin(), p.size());
	return o;
}
