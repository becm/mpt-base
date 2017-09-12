/*!
 * locate node by identifier
 */

#include <errno.h>
#include <string.h>

#include "node.h"

/*!
 * \ingroup mptNode
 * \brief get next node
 * 
 * Find node with matching identifier
 * starting with passed list element.
 * 
 * \param curr  base node
 * \param ident name of node
 */
/*@null@*/ extern MPT_STRUCT(node) *mpt_node_next(const MPT_STRUCT(node) *curr, const char *ident)
{
	size_t idlen;
	
	idlen = ident ? strlen(ident) + 1 : 0;
	
	while (curr) {
		const void *cid;
		size_t clen;
		
		cid  = mpt_identifier_data(&curr->ident);
		clen = curr->ident._len;
		if (idlen == clen
		    && !curr->ident._type
		    && (!idlen || !memcmp(ident, cid, idlen - 1))) {
			return (MPT_STRUCT(node) *) curr;
		}
		curr = curr->next;
	}
	return 0;
}
