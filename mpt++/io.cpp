/*
 * MPT C++ queue implementation
 */

#include "io.h"

__MPT_NAMESPACE_BEGIN

template <> int typeinfo<io::interface>::id()
{
    static int id = 0;
    if (!id) {
        id = mpt_type_interface_new("io");
    }
    return id;
}

// generic I/O operations
span<uint8_t> io::interface::peek(size_t)
{
    return span<uint8_t>(0, 0);
}
int64_t io::interface::pos()
{
    return -1;
}
bool io::interface::seek(int64_t )
{
    return false;
}
int io::interface::getchar()
{
    uint8_t letter;
    ssize_t rv;
    if ((rv = read(1, &letter, sizeof(letter))) < (int) sizeof(letter))
        return rv < 0 ? rv : -1;
    return letter;
}

__MPT_NAMESPACE_END
