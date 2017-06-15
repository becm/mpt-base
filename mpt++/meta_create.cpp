/*
 * MPT C++ metatype creator
 */

#include <limits>

#include <errno.h>

#include "meta.h"

#include "../mptio/stream.h"

__MPT_NAMESPACE_BEGIN

template<> int Item<metatype>::type()
{
    static int id = 0;
    if (!id) {
        id = makeItemId(metatype::Type);
    }
    return id;
}

/*!
 * \ingroup mptMeta
 * \brief create metatype
 * 
 * Create and initialize generic, buffer or special metatype.
 * 
 * \param val  initial metatype value
 * 
 * \return new metatype
 */
metatype *metatype::create(value val)
{
    const char *src = static_cast<const char *>(val.ptr);
    size_t len;

    // generic text data
    if (!val.fmt) {
        len = src ? strlen(src) : 0;
    }
    // single value payload only
    else if (!val.fmt[0] || val.fmt[1]) {
        errno = EINVAL;
        return 0;
    }
    // extended text format
    else if (!(src = mpt_data_tostring((const void **) src, *val.fmt, &len))) {
        return 0;
    }
    // dispatch to typed metatype creator
    else {
        return create(*val.fmt, val.ptr);
    }
    // compatible small generic metatype
    if (len < std::numeric_limits<uint8_t>::max()) {
        metatype *m = metatype::Basic::create(src, len);
        if (m) return m;
    }
    // create buffer-backed text metatype
    Buffer *b = new mpt::Buffer;
    b->push(strlen(static_cast<const char *>(val.ptr)), val.ptr);
    return b;
}

__MPT_NAMESPACE_END
