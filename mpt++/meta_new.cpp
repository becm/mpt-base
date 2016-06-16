/*
 * MPT C++ metatype creators
 */

#include "queue.h"
#include "array.h"
#include "node.h"

#include "object.h"

// buffer metatype override
extern "C" mpt::metatype *mpt_meta_buffer(const mpt::array *a)
{
    return new mpt::Buffer(a ? a->ref() : mpt::Reference<mpt::buffer>(0));
}
// metatype creator override
extern "C" mpt::metatype *mpt_meta_new(size_t size)
{
    mpt::metatype *m = mpt::Metatype::create(size);
    if (m) return m;
    mpt::Buffer *b = new mpt::Buffer;
    if (b) b->write(size, 0, 0);
    return b;
}
extern "C" mpt::node *mpt_node_new(size_t nlen, size_t dlen)
{ return mpt::node::create(nlen, dlen); }
