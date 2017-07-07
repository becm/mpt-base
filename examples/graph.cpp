/*!
 * instance of MPT client
 */

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include <iostream>

#include MPT_INCLUDE(layout.h)

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

int main(int , char * const [])
{
    mtrace();

    mpt::Graph *g = new mpt::Graph;

    g->set("worlds", "1 2 3");
    mpt::mpt_object_set(g, "align", "i", 5);

    g->unref();
}
