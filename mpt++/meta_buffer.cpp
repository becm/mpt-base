/*
 * MPT C++ buffer metatype creator
 */

#include "meta.h"

#include "io.h"

// buffer metatype override
extern "C" mpt::metatype *mpt_meta_buffer(const mpt::array *a)
{
	return new mpt::io::buffer(a ? *a : mpt::array(0));
}
