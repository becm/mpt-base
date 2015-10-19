/*!
 * initialize/terminate text struct.
 */

#ifndef _POSIX_C_SOURCE
# define _POSIX_C_SOURCE 200809L
#endif

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "plot.h"

/*!
 * \ingroup mptPlot
 * \brief finalize text
 * 
 * Clear resources of text data.
 * 
 * \param tx  text data
 */
extern void mpt_text_fini(MPT_STRUCT(text) *tx)
{
	if (tx->_value) free(tx->_value);
	
	if (tx->_font) free(tx->_font);
	
	tx->_font = tx->_value = 0;
}

/*!
 * \ingroup mptPlot
 * \brief initialize text structure
 * 
 * Set default text data on uninitialized memory.
 * 
 * \param tx   text data
 * \param from copy data template
 */
extern void mpt_text_init(MPT_STRUCT(text) *tx, const MPT_STRUCT(text) *from)
{
	if (from) {
		*tx = *from;
		
		if (from->_font) tx->_font = strdup(from->_font);
		if (from->_value) tx->_value = strdup(from->_value);
		
		return;
	}
	mpt_color_set(&tx->color, 0, 0, 0);
	
	tx->pos.x = tx->pos.y = 0.5;
	
	tx->style = tx->weight = 'n';
	
	tx->size = 10;
	tx->align = '5';
	
	tx->angle = 0.;
	
	tx->_font = tx->_value = 0;
}

