/*
 * MPT C++ node creator
 */

#include "node.h"

extern "C" mpt::node *mpt_node_new(size_t nlen, const mpt::value *val)
{
    if (!val) return mpt::node::create(0, nlen);
    return mpt::node::create(nlen, *val);
}
