
#include <string.h>
#include <errno.h>

#include "node.h"
/*!
 * \ingroup mptNode
 * \~english
 * \brief find child node
 * 
 * Search for child node called 'name'
 * on ordered position.
 * 
 * \param parent  root node
 * \param name    child identifier
 * \param pos     child position
 */
extern MPT_STRUCT(node) *mpt_node_find(const MPT_STRUCT(node) *parent, const char *name, int pos)
{
	MPT_STRUCT(node) *tmp;
	int idlen;
	
	if (!parent || !(tmp = parent->children)) {
		errno = EINVAL;
		return NULL;
	}
	idlen = name ? strlen(name) : 0;
	
	if (pos >= 0) {
		return mpt_node_locate(tmp, pos, name, idlen, -1);
	}
	if (!(tmp = mpt_node_locate(tmp, 0, name, idlen, -1))) {
		return 0;
	}
	return mpt_node_locate(tmp, pos, name, idlen, -1);
}

