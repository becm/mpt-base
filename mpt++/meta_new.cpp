/*
 * MPT C++ metatype creator
 */

#include "meta.h"

// metatype creator override
extern "C" mpt::metatype *mpt_meta_new(mpt::value val)
{
    return mpt::metatype::create(val);
}

