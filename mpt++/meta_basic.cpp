/*
 * MPT C++ metatype creator
 */

#include <limits>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <sys/uio.h>

#include "meta.h"

__MPT_NAMESPACE_BEGIN

metatype::Basic::Basic(size_t post)
{
    _mpt_geninfo_init(this + 1, post);
}
void metatype::Basic::unref()
{
    free(this);
}
int metatype::Basic::conv(int type, void *ptr) const
{
    return _mpt_geninfo_conv(this + 1, type, ptr);
}
metatype::Basic *metatype::Basic::clone() const
{
    struct iovec vec;
    if (_mpt_geninfo_conv(this + 1, MPT_value_toVector('c') , &vec) <= 0) {
        errno = EINVAL;
        return 0;
    }
    return create(static_cast<const char *>(vec.iov_base), vec.iov_len);
}
bool metatype::Basic::set(const char *src, int len)
{
    return _mpt_geninfo_set(this + 1, src, len) >= 0;
}

/*!
 * \ingroup mptMeta
 * \brief create basic metatype
 * 
 * Create and initialize generic, buffer or special metatype.
 * 
 * \param val  initial metatype value
 * 
 * \return new metatype
 */
metatype::Basic *metatype::Basic::create(const char *src, int len)
{
    if (len < 0) {
        len = src ? strlen(src) : 0;
    }
    int post;
    if ((post = _mpt_geninfo_size(len)) < 0) {
        errno = EINVAL;
        return 0;
    }
    Basic *m;
    void *ptr;
    if (!(ptr = malloc(sizeof(*m) + post))) {
        return 0;
    }
    m = new (ptr) Basic(post);
    m->set(src, len);
    return m;
}

__MPT_NAMESPACE_END
