
#ifndef MPT_INCLUDE
# define MPT_INCLUDE(x) <mpt/x>
#endif

#include "../module.h"

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

int main()
{
	return 0;
}
 
