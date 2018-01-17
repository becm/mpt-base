
#include <errno.h>
#include <stdlib.h>

#include "node.h"
#include "meta.h"

#include "config.h"

/*!
 * \ingroup mptConfig
 * \brief query config element
 * 
 * Find element in configuration tree.
 * Check for path.len to determine
 * if returned element is final.
 * 
\code
node = mpt_node_query(conf, path);
if (path->len) {
    // more elements in path
    …
}
\endcode
 * 
 * \param conf  configuration root
 * \param path  element position
 * 
 * \return element matching consumed path
 */
extern MPT_STRUCT(node) *mpt_node_query(const MPT_STRUCT(node) *conf, MPT_STRUCT(path) *path)
{
	MPT_STRUCT(node) *pre;
	const char *curr;
	size_t len, off;
	int clen;
	
	/* missing path information */
	if (!conf || !path->len) {
		return 0;
	}
	/* start of first path segment */
	curr = path->base + path->off;
	/* initial valid path */
	pre = 0;
	off = path->off;
	len = path->len;
	
	/* get next path element */
	while ((clen = mpt_path_next(path)) >= 0) {
		/* path element missing */
		if (!(conf = mpt_node_locate(conf, 1, curr, clen, -1))) {
			/* restore previous path parameters */
			path->off = off;
			path->len = len;
			return pre;
		}
		/* use new base nodes */
		pre = (MPT_STRUCT(node) *) conf;
		if (!(conf = conf->children)) {
			return pre;
		}
		/* start for next path segment */
		curr = path->base + path->off;
		/* save valid path state */
		off = path->off;
		len = path->len;
	}
	return pre;
}

