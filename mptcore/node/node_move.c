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
	MPT_STRUCT(node) *last = dst;
	size_t move = 0;
	
	/* move unchanged old configuration */
	while (src) {
		MPT_STRUCT(node) *curr;
		const void *id;
		int len;
		
		id  = mpt_identifier_data(&src->ident);
		len = mpt_identifier_len(&src->ident);
		
		/* move complete node */
		if (!(curr = mpt_node_locate(dst, 1, id, len))) {
			curr = src;
			src = src->next;
			mpt_node_unlink(curr);
			mpt_gnode_add(last, 0, curr);
			last = curr;
			++move;
			continue;
		}
		/* move node children */
		if (src->children) {
			/* merge children */
			if (curr->children) {
				move += mpt_node_move(src->children, curr->children);
			}
			/* reparent children to target */
			else {
				MPT_STRUCT(node) *tmp = src->children;
				
				curr->children = tmp;
				while (tmp) {
					tmp->parent = curr;
					tmp = tmp->next;
					++move;
				}
			}
		}
		src = src->next;
	}
	return move;
}
