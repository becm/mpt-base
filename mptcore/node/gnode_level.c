/*!
 * get next node on same level or first node of sublevel.
 * restrict number of accessed parent levels.
 */

#include <errno.h>

#include "node.h"

extern MPT_STRUCT(node) *mpt_gnode_samelevel(MPT_STRUCT(node) *start, size_t up)
{
	if ( !start ) {
		errno = EFAULT; return 0;
	}
	
	if ( !up || start->next )
		return start->next;
	
	start = start->parent;
	
	while ( (start = mpt_gnode_samelevel(start, up-1)) )
		if ( start->children )
			return start->children;
	return 0;
}

extern MPT_STRUCT(node) *mpt_gnode_sublevel(MPT_STRUCT(node) *start, size_t up)
{
	while ( start ) {
		if ( start->children )
			return start->children;
		
		start = mpt_gnode_samelevel(start, up);
	}
	return start;
}
