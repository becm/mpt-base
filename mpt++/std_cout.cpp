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
	if (!v.fmt) {
		if (v.ptr) o << static_cast<const char *>(v.ptr);
		return o;
	}
	if (*v.fmt) mpt_tostring(&v, writeOutStream, &o);
	return o;
}
template <> std::ostream &operator<< <char>(std::ostream &o, mpt::span<char> p)
{
	writeOutStream(&o, p.begin(), p.size());
	return o;
}
template <> std::ostream &operator<< <const char>(std::ostream &o, mpt::span<const char> p)
{
	writeOutStream(&o, p.begin(), p.size());
	return o;
}
