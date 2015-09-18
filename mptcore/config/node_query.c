
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
node = mpt_node_query(conf, path, 5);
if (path->len) node = mpt_node_get(node, path);
\endcode
 * 
 * \param conf  configuration root
 * \param path  element position
 * \param vlen  required value length
 * 
 * \return root of created elements
 */
extern MPT_STRUCT(node) *mpt_node_query(MPT_STRUCT(node) *conf, MPT_STRUCT(path) *path, size_t vlen)
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
		/* path element found */
		if ((match = mpt_node_locate(conf, 1, curr, clen))) {
			parent = match; conf = match->children;
			curr = base + path->off;
			continue;
		}
		/* create node for current path element */
		if (!(match = mpt_node_new(clen, path->len ? 0 : vlen))) {
			return 0;
		}
		if (clen && !mpt_identifier_set(&match->ident, curr, clen)) {
			mpt_node_destroy(match);
			return 0;
		}
		/* create subelements for current path */
		if (path->len) {
			MPT_STRUCT(path) tmp = *path;
			MPT_STRUCT(node) *sub;
			
			if (!(sub = mpt_node_query(0, &tmp, vlen))) {
				(void) mpt_node_destroy(match);
				return 0;
			}
			match->children = sub;
			sub->parent = match;
		}
		/* append/insert created path element */
		if (conf) {
			mpt_gnode_add(conf, 0, match);
		}
		else if (parent) {
			match->parent = parent;
			parent->children = match;
		}
		return match;
	}
	if (vlen && !match->_meta) {
		match->_meta = mpt_meta_new(vlen);
	}
	return match;
}

