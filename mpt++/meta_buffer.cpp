/*
 * MPT C++ buffer metatype creator
 */

#include "meta.h"
#include "queue.h"

#include "../mptio/stream.h"

// buffer metatype override
extern "C" mpt::iterator *mpt_meta_buffer(const mpt::array *a)
{
    return new mpt::Buffer(a ? *a : mpt::array(0));
}
