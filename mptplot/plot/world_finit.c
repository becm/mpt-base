/*!
 * default world values
 */

#include <string.h>
#include <stdlib.h>

#include "plot.h"

/*!
 * \ingroup mptPlot
 * \brief initialize world structure
 * 
 * Set default values for world members.
 * 
 * \param world  world data
 * \param from   copy data template
 */
extern void mpt_world_init(MPT_STRUCT(world) *world, const MPT_STRUCT(world) *from)
{
	if (from) {
		*world = *from;
		
		if (world->_alias) world->_alias = strdup(world->_alias);
		
		return;
	}
	world->_alias = 0;
	
	mpt_color_set(&world->color, 0, 0, 0);
	mpt_lattr_set(&world->attr, -1, -1, -1, -1);
	
	world->cyc = 0;
}

/*!
 * \ingroup mptPlot
 * \brief finalize world structure
 * 
 * Clear allocated resources.
 * 
 * \param world  world data
 */
extern void mpt_world_fini(MPT_STRUCT(world) *world)
{
	free(world->_alias);
	world->_alias = 0;
}
