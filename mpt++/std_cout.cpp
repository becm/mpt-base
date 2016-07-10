/*!
 * MPT C++ interfaces to output stream
 */

#include "convert.h"

static int writeOutStream(void *p, const char *str, size_t len)
{
    static_cast<std::ostream *>(p)->write(str, len);
    return 0;
}

std::ostream &operator<<(std::ostream &o, const mpt::value v)
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
std::ostream &operator<<(std::ostream &o, const mpt::property &p)
{
    if (!p.name) return o;
    o << p.name;
    o << " = ";
    o << p.val;
    return o;
}
template <> std::ostream &operator<< <char>(std::ostream &o, mpt::Slice<char> p)
{
    writeOutStream(&o, p.base(), p.length());
    return o;
}
template <> std::ostream &operator<< <const char>(std::ostream &o, mpt::Slice<const char> p)
{
    writeOutStream(&o, p.base(), p.length());
    return o;
}
