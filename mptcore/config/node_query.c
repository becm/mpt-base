
#include <errno.h>
#include <stdlib.h>

#include "node.h"
#include "meta.h"

#include "config.h"

/*!
 * \ingroup mptConfig
 * \brief query config element
 * 
 * Find/create element in configuration tree.
 * Check for path.len to determine if returned element
 * is final:
\code
node = mpt_node_query(conf, path, val);
if (path->len) node = mpt_node_query(node, path, 0);
\endcode
 * 
 * \param conf  configuration root
 * \param path  element position
 * \param vlen  required value length
 * 
 * \return root of created elements
 */
extern MPT_STRUCT(node) *mpt_node_query(MPT_STRUCT(node) *conf, MPT_STRUCT(path) *path, const MPT_STRUCT(value) *val)
{
	MPT_STRUCT(node) *match, *parent;
	const char *base, *curr;
	int clen;
	
	/* missing path information */
	if (!path->len) {
		if (val) {
			errno = EINVAL;
		}
		return 0;
	}
	parent = 0;
	match = 0;
	
	base = path->base;
	curr = base + path->off;
	
	/* get next path element */
	while ((clen = mpt_path_next(path)) >= 0) {
		/* path element found */
		if ((match = mpt_node_locate(conf, 1, curr, clen, -1))) {
			parent = match; conf = match->children;
			curr = base + path->off;
			continue;
		}
		if (!val) {
			return 0;
		}
		/* create node for current path element */
		if (!(match = mpt_node_new(clen))) {
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
			
			if (!(sub = mpt_node_query(0, &tmp, val))) {
				mpt_node_destroy(match);
				return 0;
			}
			match->children = sub;
			sub->parent = match;
		}
		else if (!(match->_meta = mpt_meta_new(*val))) {
			mpt_node_destroy(match);
			return 0;
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
	/* create/replace node data */
	if (val
	    && match
	    && (clen = mpt_node_set(match, val))) {
		return 0;
	}
	return match;
}

