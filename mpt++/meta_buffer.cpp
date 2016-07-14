/*
 * MPT C++ buffer metatype creator
 */

#include "meta.h"
#include "queue.h"

#include "stream.h"

// buffer metatype override
extern "C" mpt::metatype *mpt_meta_buffer(const mpt::array *a)
{
    return new mpt::Buffer(a ? a->ref() : mpt::Reference<mpt::buffer>(0));
}
