/*!
 * MPT C++ stream implementation
 */

#include "convert.h"

#include "stream.h"

__MPT_NAMESPACE_BEGIN

// streaminfo access
streaminfo::~streaminfo()
{
    _mpt_stream_setfile(this, -1, -1);
}
bool streaminfo::set_flags(int fl)
{
    if (fl & ~0xf0) return false;
    _fd |= fl;
    return true;
}

// stream data operations
stream::stream()
{ }
stream::~stream()
{
    mpt_stream_close(this);
}
int stream::flags() const
{
    return mpt_stream_flags(&this->_info) & 0xff;
}
int stream::errors() const {
    return MPT_stream_errors(mpt_stream_flags(&this->_info));
}
void stream::set_error(int err)
{
    return mpt_stream_seterror(&this->_info, err);
}
bool stream::endline()
{
    int nl = (mpt_stream_flags(&this->_info) & 0xc000) >> 14;
    const char *endl = mpt_newline_string(nl);
    if (!endl) {
        return false;
    }
    return mpt_stream_write(this, 1, endl, strlen(endl));
}
void stream::set_newline(int nl, int what)
{
    return mpt_stream_setnewline(&this->_info, nl, what);
}
bool stream::open(const char *fn, const char *mode)
{
    if (!fn) { mpt_stream_close(this); return true; }
    return mpt_stream_open(this, fn, mode) < 0 ? false : true;
}

__MPT_NAMESPACE_END
