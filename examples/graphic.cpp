/*!
 * instance of MPT graphic
 */

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include MPT_INCLUDE(graphic.h)
#include MPT_INCLUDE(layout.h)

int main()
{
    mpt::Graphic g;
    mpt::Layout *l;
    
    l = g.createLayout();
    
    l->set("name", "lay");
}
