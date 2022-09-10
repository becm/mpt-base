/*!
 * MPT C++ library
 *   interfaces to output stream
 */

#include "convert.h"

static ssize_t writeOutStream(void *p, const char *str, size_t len)
{
	static_cast<std::ostream *>(p)->write(str, len);
	return len;
}

std::ostream &operator<<(std::ostream &o, const mpt::value &v)
{
	if (v.type_id() == 's') {
		const char *str = v.string();
		if (str) o << str;
		return o;
	}
	if (!v.type_id() || mpt_tostring(&v, writeOutStream, &o) < 0) {
		o.setstate(std::ios::badbit | std::ios::failbit);
	}
	return o;
}
std::ostream &operator<<(std::ostream &o, mpt::convertable &conv)
{
	mpt::value v;
	v = &conv;
	return o << v;
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
