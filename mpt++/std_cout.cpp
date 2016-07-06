/*!
 * MPT C++ interfaces to output stream
 */

#include "convert.h"

static int writeOutStream(void *p, const char *str, size_t len)
{
    static_cast<std::basic_ostream<char> *>(p)->write(str, len);
    return 0;
}

std::basic_ostream<char> &operator<<(std::basic_ostream<char> &o, const mpt::value &v)
{
    if (!v.fmt) {
        if (v.ptr) o << static_cast<const char *>(v.ptr);
        return o;
    }
    if (!*v.fmt) return o;
    if (v.fmt[1]) o << '{';
    mpt_tostring(&v, writeOutStream, &o);
    if (v.fmt[1]) o << '}';
    return o;
}
std::basic_ostream<char> &operator<<(std::basic_ostream<char> &o, const mpt::property &p)
{
    if (!p.name) return o;
    o << p.name;
    o << " = ";
    o << p.val;
    return o;
}
