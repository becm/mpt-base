/*!
 * locate node by identifier
 */

#include <errno.h>
#include <string.h>

#include "node.h"

/*!
 * \ingroup mptNode
 * \brief locate node
 * 
 * Find node with matching identifier
 * relative to base node.
 * 
 * as 'match' on position 'pos'
 *   =0 -> last in list
 *   <0 -> previous # of same identifier
 *   >0 -> next #, starting with current = 1
 * 
 * \param curr  base node
 * \param pos   relative position to base node
 * \param ident name of node
 * \param len   length of node name or negative key type
 */
/*@null@*/extern MPT_STRUCT(node) *mpt_node_locate(const MPT_STRUCT(node) *curr, int pos, const void *ident, size_t len, int type)
{
	const char *cid;
	size_t clen, idlen;
	
	/* no matching storage -> no matching node */
	if (!curr || (len && !ident)) {
		errno = EFAULT;
		return 0;
	}
	/* existing identifier type */
	if (type >= 0) {
		idlen = len;
	}
	/* base identifier */
	else {
		type = 0;
		idlen = len + 1;
	}
	/* simple end search, check final for match */
	if (!pos) {
		while (curr->next) {
			curr = curr->next;
		}
		if (type == curr->ident._type) {
			if (type && !idlen) {
				if (curr->ident._base == ident) {
					return (MPT_STRUCT(node) *) curr;
				}
			}
			else {
				cid  = mpt_identifier_data(&curr->ident);
				clen = curr->ident._len;
				if (idlen == clen
				    && (idlen == (size_t) len || !cid[len])
				    && (!len || !memcmp(ident, cid, len))) {
					return (MPT_STRUCT(node) *) curr;
				}
			}
		}
		pos = -1;
	}
	/* negative offset, start with previous */
	if (pos < 0) {
		while ((curr = curr->prev)) {
			if (type != curr->ident._type) {
				continue;
			}
			if (type && !idlen) {
				if (!curr->ident._len
				    && curr->ident._base == ident
				    && !(++pos)) {
					break;
				}
				continue;
			}
			cid  = mpt_identifier_data(&curr->ident);
			clen = curr->ident._len;
			if (idlen == clen
			    && (idlen == (size_t) len || !cid[len])
			    && (!len || !memcmp(ident, cid, len))) {
				if (!(++pos)) {
					break;
				}
			}
		}
		return (MPT_STRUCT(node) *) curr;
	}
	/* positive offset, start with current */
	do {
		if (type != curr->ident._type) {
			continue;
		}
		if (type && !idlen) {
			if (curr->ident._base == ident
			    && !(--pos)) {
				break;
			}
			continue;
		}
		cid  = mpt_identifier_data(&curr->ident);
		clen = curr->ident._len;
		if (idlen == clen
		    && (idlen == (size_t) len || !cid[len])
		    && (!len || !memcmp(ident, cid, len))) {
			if (!(--pos)) {
				break;
			}
		}
	} while ((curr = curr->next));
	
	return (MPT_STRUCT(node) *) curr;
}
