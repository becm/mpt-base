/*
 * MPT C++ metatype creator
 */

#include "meta.h"
#include "queue.h"

#include "stream.h"

// metatype creator override
extern "C" mpt::metatype *mpt_meta_new(size_t size)
{
    mpt::metatype *m = mpt::Metatype::create(size);
    if (m) return m;
    mpt::Buffer *b = new mpt::Buffer;
    if (b) b->prepare(size);
    return b;
}
