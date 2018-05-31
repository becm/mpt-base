/*!
 * instance of MPT graphic
 */

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(graphic.h)
#include MPT_INCLUDE(layout.h)

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

int main()
{
    mtrace();
    
    mpt::Graphic g;
    mpt::Layout *l;
    
    l = g.create_layout();
    
    l->set("name", "lay");
    
    int pos = g.add_layout(l);
    g.register_update(l, mpt::UpdateHint(pos));
    
    g.remove_layout(l);
}
