/*
 * MPT C++ metatype creator
 */

#include <limits>

#include <errno.h>

#include "meta.h"

#include "../mptio/stream.h"

__MPT_NAMESPACE_BEGIN

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
    /* simple empty value */
    else if (!val.fmt[0]) {
        return metatype::Basic::create(0, 0);
    }
    // single value payload only
    else if (val.fmt[1]) {
        errno = EINVAL;
        return 0;
    }
    // extended text format
    else if (!(src = mpt_data_tostring((const void **) &src, *val.fmt, &len))) {
        // dispatch to typed metatype creator
        return create(*val.fmt, val.ptr);
    }
    // compatible small generic metatype
    if (len < std::numeric_limits<uint8_t>::max()) {
        metatype *m = metatype::Basic::create(src, len);
        if (m) return m;
    }
    // create buffer-backed text metatype
    Buffer *b = new mpt::Buffer;
    b->push(len, src);
    return b;
}

__MPT_NAMESPACE_END
