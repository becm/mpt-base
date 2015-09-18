/*!
 * move non-present nodes only
 */

#include "node.h"

/*!
 * \ingroup mptNode
 * \brief move node elements
 * 
 * recursive move of list elements to target list.
 * only move unset elements.
 * 
 * \param src  source nodes list
 * \param dst  target nodes list
 * 
 * \return number of moved elemets
 */
size_t mpt_node_move(MPT_STRUCT(node) *src, MPT_STRUCT(node) *dst)
{
	size_t move = 0;
	
	/* move unchanged old configuration */
	for ( ; src; src = src->next) {
		MPT_STRUCT(node) *curr;
		const void *id;
		int len;
		
		id  = mpt_identifier_data(&src->ident);
		len = mpt_identifier_len(&src->ident);
		if ((curr = mpt_node_locate(dst, 1, id, len))) {
			if (src->children && curr->children) {
				move += mpt_node_move(curr->children, src->children);
			}
			continue;
		}
		mpt_node_unlink(src);
		mpt_gnode_add(dst, 0, src);
		++move;
	}
	return move;
}
