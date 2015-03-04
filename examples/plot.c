#include <stdio.h>
#include <stdlib.h>

#include <mpt/layout.h>

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

int main()
{
	MPT_STRUCT(axis) ax;
	MPT_STRUCT(property) pr;
	
	pr.name = "begin";
	mpt_axis_init(&ax, 0);
	mpt_axis_pget(&ax, &pr, 0);
	
	return 0;
}
