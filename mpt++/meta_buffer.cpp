/*
 * MPT C++ buffer metatype creator
 */

#include "queue.h"
#include "array.h"

// buffer metatype override
extern "C" mpt::metatype *mpt_meta_buffer(const mpt::array *a)
{
    return new mpt::Buffer(a ? a->ref() : mpt::Reference<mpt::buffer>(0));
}
