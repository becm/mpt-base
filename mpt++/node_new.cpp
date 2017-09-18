/*
 * MPT C++ node creator
 */

#include "node.h"

extern "C" mpt::node *mpt_node_new(size_t nlen)
{
    return mpt::node::create(nlen);
}
