
#include <errno.h>
#include <stdlib.h>

#include "node.h"
#include "config.h"

/*!
 * \ingroup mptConfig
 * \brief query config element
 * 
 * Find/create element in configuration tree.
 * Check for path.len to determine if returned element
 * is final:
\code
node = mpt_node_query(conf, path);
if (path->len) node = mpt_node_get(node, path);
\endcode
 * 
 * \param conf  configuration root
 * \param path  element position
 * 
 * \return root of created elements
 */
extern MPT_STRUCT(node) *mpt_node_query(MPT_STRUCT(node) *conf, MPT_STRUCT(path) *path)
{
	MPT_STRUCT(node) *match, *parent;
	const char *base, *curr;
	int clen;
	
	/* missing path information */
	if (!path->len) {
		errno = EINVAL;
		return 0;
	}
	parent = 0;
	match = 0;
	
	base = path->base;
	curr = base + path->off;
	
	/* get next path element */
	while ((clen = mpt_path_next(path)) >= 0) {
		size_t vlen;
		/* path element found */
		if ((match = mpt_node_locate(conf, 1, curr, clen))) {
			parent = match; conf = match->children;
			curr = base + path->off;
			continue;
		}
		vlen = path->len ? 0 : path->valid;
		
		/* create node for current path element */
		if (!(match = mpt_node_new(vlen, curr, clen))) {
			return 0;
		}
		/* create subelements for current path */
		if (path->len) {
			MPT_STRUCT(path) tmp = *path;
			
			if (!(match->children = mpt_node_query(0, &tmp))) {
				(void) mpt_node_destroy(match);
				return 0;
			}
			match->children->parent = match;
		}
		/* append/insert created path element */
		if (conf) {
			mpt_gnode_add(conf, 0, match);
		}
		else if (parent) {
			match->parent = parent;
			parent->children = match;
		}
		break;
	}
	
	return match;
}

