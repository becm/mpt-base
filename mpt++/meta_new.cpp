/*
 * MPT C++ metatype creator
 */

#include "queue.h"
#include "array.h"

// metatype creator override
extern "C" mpt::metatype *mpt_meta_new(size_t size)
{
    mpt::metatype *m = mpt::Metatype::create(size);
    if (m) return m;
    mpt::Buffer *b = new mpt::Buffer;
    if (b) b->write(size, 0, 0);
    return b;
}
