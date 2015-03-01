/*!
 * default graph values
 */

#include <string.h>
#include <stdlib.h>

#include "layout.h"

/*!
 * \ingroup mptLayout
 * \brief finalize graph
 * 
 * Clear resources of graph data.
 * 
 * \param gr graph data
 */
extern void mpt_graph_fini(MPT_STRUCT(graph) *gr)
{	
	free(gr->_axes);
	gr->_axes = 0;
	
	free(gr->_worlds);
	gr->_worlds = 0;
}

/*!
 * \ingroup mptLayout
 * \brief initialize graph structure
 * 
 * Set default values for graph members.
 * 
 * \param gr   uninitialized graph data
 * \param from graph data template
 */
extern void mpt_graph_init(MPT_STRUCT(graph) *gr, const MPT_STRUCT(graph) *from)
{
	if (from) {
		*gr = *from;
		
		if (gr->_axes) gr->_axes = strdup(gr->_axes);
		if (gr->_worlds) gr->_worlds = strdup(gr->_worlds);
		
		return;
	}
	gr->_axes = 0;
	gr->_worlds = 0;
	
	mpt_color_set(&gr->fg,0,0,0);
	mpt_color_set(&gr->bg,0xff,0xff,0xff);
	mpt_color_setalpha(&gr->bg,0);
	
	gr->pos.x = gr->pos.y = .0;
	gr->scale.x = gr->scale.y = 1.0;
	
	gr->grid = gr->frame = 0;
	
	gr->align = MPT_ENUM(AlignBegin) | MPT_ENUM(AlignBegin) << 2 | MPT_ENUM(AlignBegin) << 4;
	gr->clip = 0;
	
	gr->lpos = 0;
}
